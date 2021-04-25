#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>

#include "qtcp.h"
#include "dom32iowifi.h"

int main(int argc, char** argv, char** env)
{
    int i;
    Dom32IoWifi *pD32W;
    char *addr = NULL;
    char *command = NULL;
    int status;

    for(i = 1; i < argc; i++)
    {
        if( !strcmp(argv[i], "-a") || !strcmp(argv[i], "--addr"))
        {
            i++;
            addr = argv[i];
        }
        else if( !strcmp(argv[i], "--iostatus"))
        {
            command = "iostatus";
        }
        else if( !strcmp(argv[i], "--exstatus"))
        {
            command = "exstatus";
        }
        else if( !strcmp(argv[i], "--ioconfig"))
        {
            command = "ioconfig";
        }
        else if( !strcmp(argv[i], "--exconfig"))
        {
            command = "exconfig";
        }
    }


    pD32W = new Dom32IoWifi();


    pD32W->GetIOStatus(addr, &status);


    printf("Status: 0x%04X\n", status);


    return 0;
}
