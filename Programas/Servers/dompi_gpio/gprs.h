																																																																																																																																												/* ************************************************************************** *
 *  Project:            uCom
 *  File:				message.h
 *  Date:               1 de Agosto de 2019
 *  Author:             Walter Pirri
 *                      (walter***AT***pirri***DOT***com***DOT***ar)
 *  Company:            SIIA
 * ************************************************************************** */
#ifndef _CGSM_H_
#define	_CGSM_H_

#include "main.h"

#include <time.h>

/* Definiciones del indicador de señal */
#define GPRS_SIGNAL_DELTA	3
#define GPRS_SIGNAL_PERIOD	(GPRS_SIGNAL_DELTA*17)

void GprsInit( void );

void GprsTasks( void );
void GprsTimer( void );

void GprsReceive( void );

int GprsReady( void );

int GprsAlarma(const char* event, int part, int zone, const char *t);
void GprsGetTime( void );
void GprsGetRSSI( void );
void GprsSendConfig( void );
void GprsCommand( const char *cmd );

#endif	/* _CGSM_H_ */

