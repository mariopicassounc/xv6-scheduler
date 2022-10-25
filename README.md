# Informe Laboratorio 3: Planificación

Grupo 22:
 - Mario Picasso
 - Mateo Malpassi
 - Facundo Coria
 - Bruno Espinosa

## Primera Parte: Estudiando el planificador de xv6 

### Ejercicio 1

>Analizar el código del planificador y responda: ¿Qué política de planificación utiliza xv6 para elegir el próximo proceso a ejecutarse? Pista: xv6 nunca sale de la función scheduler por medios “normales”.

La política utilizada es Round Robin, esta política trata a todos los procesos del sistema operativo con la misma prioridad y les da a todos un tiempo máximo de uso de CPU  denominado “quantum”. 

Cada vez que un quantum termina, se produce un timer interrupt, éste se identifica en la función usertrap() del archivo trap.c. Luego se llama yield() -en proc.c-, donde se cambia el estado del proceso de RUNNING a RUNNABLE y luego dentro de esta se llama a sched(), donde se realiza el cambio de contexto al scheduler mediante la función swtch().

Notar que el scheduler se ejecuta infinitamente, y se vuelve a este cada vez que un proceso se deja de ejecutar ya sea por timer interrupt, io, o syscall, dejando de ejecutarse sólo en caso que ocurra alguna excepción. Esta función lo que hace es recorrer la tabla de procesos buscando alguno que esté en estado RUNNABLE y lo cambia a RUNNING, luego hace el cambio de contexto al proceso mediante swtch().

Así, este planificador va alternando entre proceso y proceso sin importar si estos son cpubound o iobound. Por lo que tiene buen tiempo de respuesta pero no es bueno para la interacción. 


### Ejercicio 2

#### Apartado a

>¿Cuánto dura un quantum en xv6?
```c
// ask the CLINT for a timer interrupt.
 int interval = 1000000; // cycles; about 1/10th second in qemu.
```
Se define en start.c. Como menciona dura 1000000 interrupciones, que según el comentario son 1/10 de segundo en el qemu.

#### Apartado b

>¿Cuánto dura un cambio de contexto en xv6?

Es bastante rápido. Si miramos el archivo swtch.S son 14 operaciones de guardar un valor de registro en memoria y 14 de cargar un valor de memoria en un registro. 28 operaciones de transferencia.


#### Apartado c

>¿El cambio de contexto consume tiempo de quantum?

En xv6 las interrupciones por tiempo están desactivadas cuando se pasa a kernel space, el context switch definitivamente no consume tiempo de quantum.

#### Apartado d

>¿Hay alguna forma de que a un proceso se le asigne menos tiempo?

Si habilitamos las device interrupts sí se puede cortar un proceso a mitad quantum y switchear al planificador.

## Segunda Parte: Cómo el planificador afecta los procesos 

### Mediciones, ¿Cómo influye cada planificador?

La diferencia entre Round Robin y MLFQ es cómo asigna cada uno de estos los recursos que se le asignan a cada proceso. En el caso de Round Robin, se le asignan por igual la cantidad de recursos a todos los procesos; en cambio, MLFQ asigna más o menos recursos según en el nivel de la cola que se encuentre el proceso.

Para hacer las respectivas comparaciones, se utilizaron dos programas: iobench y cpubench.

Cpubench mide la cantidad de operaciones de punto flotante que se realizan en el sistema según una cantidad fija de ticks que se establecen antes de ejecutar el programa.

Iobench, por otro lado, mide la cantidad de escritura y lectura de un archivo que se hace en una determinada cantidad de ticks.

Los ticks se establecen como una constante en ambos programas denominada “MINTICKS”.

### Realización de tests

Todos los tests se realizaron en el mismo PC, para así ser posible comparar resultados. Luego, cada test fue ejecutado por 5 minutos y se sacó el promedio de todos los valores que arrojó cada uno de estos. Con estos valores, luego se graficó.

En cada gráfico se especifica qué mediciones se hicieron con Round Robin, y qué mediciones se realizaron con MLFQ.

Los tests de iobench y de cpubench los graficamos separados, para que el gráfico quede más limpio y sea más fácil de interpretar (ya que cpubench imprime los resultados en KFLOPT, e iobench los imprime en IOP).

Por último, para los escenarios en donde el quantum debe ser reducido, lo que hicimos fue: por cada 0 que le sacamos al quantum, le agregamos un 0 a MINTICKS en iobench y cpubench respectivamente para que de esta manera los programas corran durante la misma cantidad de tiempo antes de imprimir un resultado. 

A continuación, los resultados y el análisis general de lo que calculamos:

#### Round Robin

##### Escenario 0: quantum por defecto

![RR Cpu Q=Default](/testsGráficos/RRcpudefault.jpg)

![RR IO Q=Default](/testsGráficos/RRiodefault.jpg)

##### Escenario 1: quantum 10 veces más corto

![RR Cpu Q=div 10](/testsGráficos/RRcpudiv10.jpg)

![RR IO Q=div 10](/testsGráficos/RRiodiv10.jpg)

##### Escenario 2: quantum 100 veces más corto

![RR Cpu Q=div 100](/testsGráficos/RRcpudiv100.jpg)

![RR IO Q=div 100](/testsGráficos/RRiodiv100.jpg)

##### Escenario 3: quantum 1000 veces más corto

![RR Cpu Q=div 1000](/testsGráficos/RRcpudiv1000.jpg)

![RR IO Q=div 100](/testsGráficos/RRiodiv1000.jpg)

##### Análisis final Round Robin:

Luego de realizar los tests y hacer los gráficos, notamos principalmente un comportamiento que no debería ser el habitual: particularmente, en los tests donde se ejecutan 2 procesos a la vez (ya sea 2 iobench, o 2 cpubench) lo que debería pasar es que la cantidad de operaciones en estos ejemplos sea menor que cuando ejecutamos sólo 1 de estos procesos, no al revés como sucede en este caso. Al parecer el qemu estaría haciendo una optimización brindando más poder de cómputo.

Otra cosa a recalcar es la siguiente: En el escenario 4, que se opera con un quantum 1000 veces más corto, lo que sucede es que se pasa tanto tiempo haciendo context-switch que cpubench no llega a imprimir ningún resultado.

#### MLFQ

##### Escenario 0: quantum por defecto

![MLFQ Cpu Q=Default](/testsGráficos/MLFQcpudefault.png)

![MLFQ IO Q=Default](/testsGráficos/MLFQiodefault4.png)

##### Escenario 1: quantum 10 veces más corto

![MLFQ Cpu Q=div 10](/testsGráficos/MLFQcpudiv10.png)

![MLFQ IO Q=div 10](/testsGráficos/MLFQiodiv10.png)

##### Escenario 2: quantum 100 veces más corto

![MLFQ Cpu Q=div 100](/testsGráficos/MLFQcpudiv100.png)

![MLFQ IO Q=div 100](/testsGráficos/MLFQiodiv100.png)

##### Escenario 3: quantum 1000 veces más corto

![MLFQ Cpu Q=div 1000](/testsGráficos/MLFQcpudiv1000.png)

![MLFQ IO Q=div 1000](/testsGráficos/MLFQiodiv1000.png)

## Tercera Parte: Implementando MLFQ
### Rastreando la prioridad de los procesos
#### MLFQ regla 3: rastreo de prioridad y asignación máxima
* La prioridad maxima es 0
* NPRIO - 1 es la prioridad minima 

Para que los procesos se inicien con la prioridad más alta se setea `p->priority = 0` en la funcion allocproc de proc.c, que es la encargada de inicializar el estado del proceso para ser ejecutado. 

#### MLFQ regla 4: descenso y ascenso de prioridad
El descenso de prioridad va a ocurrir antes de hace `yield()` en el caso `which_dev == 2` de la funcion `usertrap()` del archivo `trap.c`. Esto es así debido que allí llega la ejecución cuando se produce una interrupción por tiempo al proceso que estaba corriendo en user, i.e que ya se le terminó el quantum.

Y el ascenso de prioridad ocurre en el sleep(), ya que es la zona del codigo donde un proceso pasa a estar bloqueado.

### Implementando MLFQ

#### MLFQ regla 1: correr el proceso de mayor prioridad
Decidimos implementar una cola que se puede acceder mediante `queue_first` y `queue_last`. Además cada proceso tiene el campo `proc->next_proc` que es un puntero al próximo elemento en la cola, al igual que `proc->priority` fue agregado en la implementación de la estructura en `proc.h`

Por simplicidad se tomó el approach de que la cola tenga solo procesos RUNNABLE.

Hay 3 colas, una por cada prioridad, y para correr el proceso de mayor prioridad simplemente seria buscar a qué proceso apunta `queue_first[0]`. Si no hay nada saltamos un paso de iteración nos vamos a ver que hay en la cola de procesos de prioridad 1, y así sucesivamente.

Para que haya en las colas solo procesos en estado RUNNABLE utilizamos la función `elim_proc_in_queue(struct proc *p)` que se ejecuta antes de correr un proceso (RUNNABLE a RUNNING)

#### MLFQ regla 2: round-robin para procesos de misma prioridad
Se va a correr el proceso RUNNABLE con prioridad más alta, luego se vuelve a buscar en las colas un proceso para ejecutar. 
En el caso de que la cola era la más prioritaria el porqué es round robin es trivial.
En el caso de que se estaba ejecutando un proceso de una cola menos prioritaria, solo va a continuar en Round Robin si no hay un proceso de prioridad mas alta en estado RUNNABLE esperando en alguna cola más prioritaria.

#### Starvation
Casos en los que para mi se produciría starvation

Si tenés muchos procesos IO es probable que se produzca starvation ya que son procesos que devuelven el control al kernel antes de que se termine su quantum, y por lo tanto suben su prioridad o bien se mantienen en la prioridad más alta.

Si haces las suficientes Device Interrupts ej mandando paquetes de red podes negar la ejecución de procesos CPU bound.
Estarías cortando constantemente la ejecución de los procesos CPU sin dejaros avanzar (similar a DOS ¿?)

##### Análisis final: MLFQ comparado con Round Robin

MLFQ se comporta como esperamos: cuando ejecutamos cpu e iobench juntos, se nota claramente que se le da más prioridad a los IO que a los cpu; este es el comportamiento esperable.

El funcionamiento sería: Ejecuto la línea “cpubench &; iobench &”, luego rápidamente MLFQ asigna a cpubench en la cola de baja prioridad, y a iobench lo asigna a la cola de mayor prioridad.

Esta es la diferencia principal que tiene MLFQ con respecto a Round Robin.

Algo también a destacar es que, a medida que el quantum se va reduciendo, cpu e IO hacen cada vez menos operaciones, hasta tal punto que con quantums 100 o 1000 veces más corto, sólo imprime un par de resultados luego de ejecutarlos por 5 minutos.

Nuevamente, en algunos, casos hay comportamientos extraños en los tests usando MLFQ (al igual que usando Round Robin): 
 - El primero es un resultado incongruente: Cuando ejecutamos 2 IO, o 2 CPU, el promedio de operaciones sigue siendo casi igual que cuando ejecutamos sólo uno de cada. Esto no debería ser así,  sino que debería ser lo inversa a este comportamiento (cuando ejecuto 2 IO, por ejemplo, el promedio tendría que ser menor a cuando ejecuto sólo 1 IO).
 - El segundo resultado “extraño” sigue siendo cuando ejecutamos cpubench con quantum 1000 veces menor: Con tests de sólo 5 minutos no
 llega a imprimir ningún resultado.

