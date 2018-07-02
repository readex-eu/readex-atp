#include <stdio.h>
#include <stdlib.h>
#include "atpserver.h"

int main(int argc, char **argv)
{
    const char* regHost = NULL;
    int regPort = 0;

    if(argc < 2){
        ATP_ERROR("Missing argument to ATP server main function (host: localhost and port 20000 will be used)\n"); 
    }else{
        regHost = argv[1];
        regPort = (int) strtod(argv[2], 0);
    }
    int tmp_ = ATPS_init (regHost, regPort);
    
    return EXIT_SUCCESS;
}
