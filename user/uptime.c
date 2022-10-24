#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>

int 
main(int argc, char *argv[])
{
    for(int i = 0; i < 100; i++){
        printf("%d\n", uptime());
    }
    return 0;
}