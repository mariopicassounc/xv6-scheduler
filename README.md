# Informe Laboratorio 3: Planificación

Grupo 22:
 - Mario Picasso
 - Mateo Malpassi
 - Facundo Coria
 - Bruno Espinosa

## Primera Parte: Estudiando el planificador de xv6 

### Ejercicio 1

>Analizar el código del planificador y responda: ¿Qué política de planificación utiliza xv6 para elegir el próximo proceso a ejecutarse? Pista: xv6 nunca sale de la función scheduler por medios “normales”.

La política utilizada es Round Robin, esta política ejecuta todos los procesos del sistema operativo una determinada cantidad de tiempo denominada “quantum”. Todos los procesos son tratados con la misma prioridad. Esta política da un tiempo máximo de uso de CPU a cada proceso, una vez que este tiempo pasa, es pasado de estado “RUNNING” a estado “RUNNABLE”.

Cada vez que un quantum termina, se produce un timer interrupt, éste se identifica en la función usertrap() del archivo trap.c. Luego se llama yield() y sched(), que está en proc.c, donde se realiza el cambio de contexto al scheduler.
Es importante notar que estamos volviendo a scheduler, que es infinitamente ejecutado. 

### Ejercicio 2

#### Apartado a

>¿Cuánto dura un quantum en xv6?
``
// ask the CLINT for a timer interrupt.
 int interval = 1000000; // cycles; about 1/10th second in qemu.
``
Se define en start.c. Como menciona dura 1000000 interrupciones, que según el comentario son 1/10 de segundo.

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


##### Análisis final: MLFQ comparado con Round Robin

MLFQ se comporta como esperamos: cuando ejecutamos cpu e iobench juntos, se nota claramente que se le da más prioridad a los IO que a los cpu; este es el comportamiento esperable.

El funcionamiento sería: Ejecuto la línea “cpubench &; iobench &”, luego rápidamente MLFQ asigna a cpubench en la cola de baja prioridad, y a iobench lo asigna a la cola de mayor prioridad.

Esta es la diferencia principal que tiene MLFQ con respecto a Round Robin.

Algo también a destacar es que, a medida que el quantum se va reduciendo, cpu e IO hacen cada vez menos operaciones, hasta tal punto que con quantums 100 o 1000 veces más corto, sólo imprime un par de resultados luego de ejecutarlos por 5 minutos.

Nuevamente, en algunos, casos hay comportamientos extraños en los tests usando MLFQ (al igual que usando Round Robin): 
 - El primero es un resultado incongruente: Cuando ejecutamos 2 IO, o 2 CPU, el promedio de operaciones sigue siendo casi igual que cuando ejecutamos sólo uno de cada. Esto no debería ser así,  sino que debería ser lo inversa a este comportamiento (cuando ejecuto 2 IO, por ejemplo, el promedio tendría que ser menor a cuando ejecuto sólo 1 IO).
 - El segundo resultado “extraño” sigue siendo cuando ejecutamos cpubench con quantum 1000 veces menor: Con tests de sólo 5 minutos no
 llega a imprimir ningún resultado.

