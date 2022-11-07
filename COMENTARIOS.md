## Informe 
- Bien
- Se le puede asignar un quantum "injusto" a un proceso porque el quantum es una lectura en el reloj del hardware, en este caso emulado. El context switch si come tiempo de quantum
- Cambiaron MINTICKS?
- Las gráfcas podrían ser más sintéticas
- Hay buenos análisis, me gusta que aclararon los casos raros pero faltó un poco de hablar del porqué de algunos comportamientos. Por ejemplo por qué un iobench con un cpubench no cambia mucho la performance de cpubench (en RR)

## Repo 
- Medio despareja la autoría de commits

## Código
- OK
- Me gustó el sistema de colas que hicieron. Faltó lockear el recurso compartido

## Funcionalidad
- Bien `proc.h` y `procdump`
- Bien el RR por nivel
- Bien el rechequeo de prioridades
