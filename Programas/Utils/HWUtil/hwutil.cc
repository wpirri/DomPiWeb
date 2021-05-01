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

typedef enum {
    cmd_none = 0,
    cmd_iostatus,
    cmd_exstatus,
    cmd_ioconfig,
    cmd_exconfig,
    cmd_getconfig,
    cmd_ioset,
    cmd_ioreset,
    cmd_exset,
    cmd_exreset,
    cmd_wifiget,
    cmd_wifiset
} command;

int main(int argc, char** argv/*, char** env*/)
{
    int i;
    Dom32IoWifi *pD32W;
    Dom32IoWifi::wifi_config_data wifi_data;
    char *addr = NULL;
    command cmd = cmd_none;
    int ioval, exval;
    long lval;
    char* pval = (char*)"0";
    int showhelp = 0;
    int verbose = 0;

    pD32W = new Dom32IoWifi();

    memset(&wifi_data, 0, sizeof(Dom32IoWifi::wifi_config_data));

    for(i = 1; i < argc; i++)
    {
        if( !strcmp(argv[i], "-a") || !strcmp(argv[i], "--addr"))
        {
            i++;
            addr = argv[i];
        }
        else if( !strcmp(argv[i], "--iostatus"))
        {
            cmd = cmd_iostatus;
        }
        else if( !strcmp(argv[i], "--exstatus"))
        {
            cmd = cmd_exstatus;
        }
        else if( !strcmp(argv[i], "--ioconfig"))
        {
            cmd = cmd_ioconfig;
            i++;
            pval = argv[i];
        }
        else if( !strcmp(argv[i], "--exconfig"))
        {
            cmd = cmd_exconfig;
            i++;
            pval = argv[i];
        }
        else if( !strcmp(argv[i], "--getconfig"))
        {
            cmd = cmd_getconfig;
        }
        else if( !strcmp(argv[i], "--ioset"))
        {
            cmd = cmd_ioset;
            i++;
            pval = argv[i];
        }
        else if( !strcmp(argv[i], "--ioreset"))
        {
            cmd = cmd_ioreset;
            i++;
            pval = argv[i];
        }
        else if( !strcmp(argv[i], "--exset"))
        {
            cmd = cmd_exset;
            i++;
            pval = argv[i];
        }
        else if( !strcmp(argv[i], "--exreset"))
        {
            cmd = cmd_exreset;
            i++;
            pval = argv[i];
        }
        else if( !strcmp(argv[i], "--wifiget"))
        {
            cmd = cmd_wifiget;
        }
        else if( !strcmp(argv[i], "--wifiset"))
        {
            cmd = cmd_wifiset;
        }
        else if( !strcmp(argv[i], "-v"))
        {
            verbose = 1;
        }
        /* Parametros del wifi */
        else if( !strcmp(argv[i], "--ap1"))
        {
            i++;
            strcpy(wifi_data.wifi_ap1, argv[i]);
        }
        else if( !strcmp(argv[i], "--ap1p"))
        {
            i++;
            strcpy(wifi_data.wifi_ap1_pass, argv[i]);
        }
        else if( !strcmp(argv[i], "--ap2"))
        {
            i++;
            strcpy(wifi_data.wifi_ap2, argv[i]);
        }
        else if( !strcmp(argv[i], "--ap2p"))
        {
            i++;
            strcpy(wifi_data.wifi_ap2_pass, argv[i]);
        }
        else if( !strcmp(argv[i], "--ce1"))
        {
            i++;
            strcpy(wifi_data.wifi_host1, argv[i]);
        }
        else if( !strcmp(argv[i], "--ce2"))
        {
            i++;
            strcpy(wifi_data.wifi_host2, argv[i]);
        }
        else if( !strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            showhelp = 1;
        }
        else
        {
            printf("Parametro [%s] desconocido.\n", argv[i]);
            showhelp = 1;
        }
    }

    if( !showhelp )
    {
        if(addr == NULL)
        {
            printf("Falta el parametro obligatorio [--addr].\n ");
            showhelp = 1;
        }
        
        if(cmd == cmd_none)
        {
            printf("Falta el parametro obligatorio [--cmd].\n ");
            showhelp = 1;
        }
    }

    if(showhelp)
    {
        printf("Sintaxis:\n");
        printf("  %s [-v] -a|--addr <ip address> <command> [<command params>].\n", argv[0]);
        printf("  -v : Muestra información detallada.\n");
        printf("  <ip address> Direccion IP del dispositivo WiFi.\n");
        printf("  <command> Uno de los siguientes:\n");
        printf("      --iostatus:  Estado del port de I/O.\n");
        printf("      --exstatus:  Estado del port de expansion.\n");
        printf("      --ioconfig <config>:  Configuracion de entrasa/salida del port de I/O.\n");
        printf("      --exconfig <config>:  Configuracion de entrasa/salida del port de expansion.\n");
        printf("      --getconfig: Devuelve la configuracion de entrada/salida.\n");
        printf("      --ioset <mask>:     Enciende salidas de I/O segun la mascara.\n");
        printf("      --ioreset <mask>:   Apaga salidas de I/O segun la mascara.\n");
        printf("      --exset <mask>:     Enciende salidas de expansion segun la mascara.\n");
        printf("      --exreset <mask>:   Apaga salidas de expansion segun la mascara.\n");
        printf("      --wifiget:   Obtiene la configuracion de wifi.\n");
        printf("      --wifiset:   Cambia la configuracion de wifi.\n");
        printf("  parametros de wifi:\n");
        printf("      --ap1 <nombre>:  Primer Access Point.\n");
        printf("      --ap1p <clave>:  Clave del primer Access Point.\n");
        printf("      --ap2 <nombre>:  Segundo Access Point.\n");
        printf("      --ap2p <clave>:  Clave del segundo Access Point.\n");
        printf("      --ce1 <ip|host>: Central primaria.\n");
        printf("      --ce2 <ip|host>: Central de backup.\n");
        delete pD32W;
        return 0;
    }

    pD32W->m_verbose = verbose;
    
    switch(cmd)
    {
        case cmd_iostatus:
            pD32W->GetIOStatus(addr, &ioval);
            printf("I/O Status: 0x%02X\n", ioval);
            break;
        case cmd_exstatus:
            pD32W->GetEXStatus(addr, &exval);
            printf("Exp Status: 0x%02X\n", exval);
            break;
        case cmd_ioconfig:
            if(*pval == '0' && (*(pval+1) == 'x' ||*(pval+1) == 'X') )
            {
                lval = strtol((pval+2), NULL, 16);
            }
            else if(*pval == '0' && (*(pval+1) == 'b' ||*(pval+1) == 'B') )
            {
                lval = strtol((pval+2), NULL, 2);
            }
            else
            {
                lval = strtol(pval, NULL, 10);
            }
            pD32W->ConfigIO(addr, lval, &ioval);
            printf("I/O Config: 0x%02X\n", ioval);
            break;
        case cmd_exconfig:
            if(*pval == '0' && (*(pval+1) == 'x' ||*(pval+1) == 'X') )
            {
                lval = strtol((pval+2), NULL, 16);
            }
            else if(*pval == '0' && (*(pval+1) == 'b' ||*(pval+1) == 'B') )
            {
                lval = strtol((pval+2), NULL, 2);
            }
            else
            {
                lval = strtol(pval, NULL, 10);
            }
            pD32W->ConfigEX(addr, lval, &exval);
            printf("Exp Config: 0x%02X\n", exval);
            break;
        case cmd_getconfig:
            pD32W->GetConfig(addr, &ioval, &exval);
            printf("I/O Config: 0x%02X\n", ioval);
            printf("Exp Config: 0x%02X\n", exval);
            break;
        case cmd_ioset:
            if(*pval == '0' && (*(pval+1) == 'x' ||*(pval+1) == 'X') )
            {
                lval = strtol((pval+2), NULL, 16);
            }
            else if(*pval == '0' && (*(pval+1) == 'b' ||*(pval+1) == 'B') )
            {
                lval = strtol((pval+2), NULL, 2);
            }
            else
            {
                lval = strtol(pval, NULL, 10);
            }
            pD32W->SetIO(addr, lval, &ioval);
            printf("I/O Status: 0x%02X\n", ioval);
            break;
        case cmd_ioreset:
            if(*pval == '0' && (*(pval+1) == 'x' ||*(pval+1) == 'X') )
            {
                lval = strtol((pval+2), NULL, 16);
            }
            else if(*pval == '0' && (*(pval+1) == 'b' ||*(pval+1) == 'B') )
            {
                lval = strtol((pval+2), NULL, 2);
            }
            else
            {
                lval = strtol(pval, NULL, 10);
            }
            pD32W->ResetIO(addr, lval, &ioval);
            printf("I/O Status: 0x%02X\n", ioval);
            break;
        case cmd_exset:
            if(*pval == '0' && (*(pval+1) == 'x' ||*(pval+1) == 'X') )
            {
                lval = strtol((pval+2), NULL, 16);
            }
            else if(*pval == '0' && (*(pval+1) == 'b' ||*(pval+1) == 'B') )
            {
                lval = strtol((pval+2), NULL, 2);
            }
            else
            {
                lval = strtol(pval, NULL, 10);
            }
            pD32W->SetEX(addr, lval, &exval);
            printf("Exp Status: 0x%02X\n", exval);
            break;
        case cmd_exreset:
            if(*pval == '0' && (*(pval+1) == 'x' ||*(pval+1) == 'X') )
            {
                lval = strtol((pval+2), NULL, 16);
            }
            else if(*pval == '0' && (*(pval+1) == 'b' ||*(pval+1) == 'B') )
            {
                lval = strtol((pval+2), NULL, 2);
            }
            else
            {
                lval = strtol(pval, NULL, 10);
            }
            pD32W->ResetEX(addr, lval, &exval);
            printf("Exp Status: 0x%02X\n", exval);
            break;
        case cmd_wifiget:
            pD32W->GetWifi(addr, &wifi_data);
            printf("Wifi ap1: %s\n", wifi_data.wifi_ap1);
            printf("    pass: %s\n", wifi_data.wifi_ap1_pass);
            printf("Wifi ap2: %s\n", wifi_data.wifi_ap2);
            printf("    pass: %s\n", wifi_data.wifi_ap2_pass);
            printf("Central1: %s:%i\n", wifi_data.wifi_host1, wifi_data.wifi_host1_port);
            printf("Central2: %s:%i\n", wifi_data.wifi_host2, wifi_data.wifi_host2_port);
            printf("URI:      %s\n", wifi_data.rqst_path);
            break;
        case cmd_wifiset:
            pD32W->SetWifi(addr, &wifi_data);
            printf("Wifi ap1: %s\n", wifi_data.wifi_ap1);
            printf("    pass: %s\n", wifi_data.wifi_ap1_pass);
            printf("Wifi ap2: %s\n", wifi_data.wifi_ap2);
            printf("    pass: %s\n", wifi_data.wifi_ap2_pass);
            printf("Central1: %s:%ui\n", wifi_data.wifi_host1, wifi_data.wifi_host1_port);
            printf("Central2: %s:%ui\n", wifi_data.wifi_host2, wifi_data.wifi_host2_port);
            printf("URI:      %s\n", wifi_data.rqst_path);
            break;
        default:
            break;
    }
    delete pD32W;
    return 0;
}
