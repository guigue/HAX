

// MZ_TESTE
// ADICIONADO ALGUMAS FUNCOES PARA EVITAR DAR ERRO COMPILICAO
// FALTA: INCLUIR NetworkInterface.c codigo fonte real do PHY 8051 e acertar para 8081???


/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "NetworkInterface.h"

#include "sam4e_xplained_pro.h"
//#include "hr_gettime.h"
#include "conf_eth.h"
//#include "ksz8851snl.h"
//#include "ksz8851snl_reg.h"

/* Some files from the Atmel Software Framework */
#include <sysclk.h>
//#include <pdc/pdc.h>
//#include <spi/spi.h>


BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxNetworkBuffer, BaseType_t bReleaseAfterSend )
{
	BaseType_t xResult = pdFALSE;
	return xResult;
}
/*-----------------------------------------------------------*/


BaseType_t xNetworkInterfaceInitialise( void )
{
	return 0;
}
