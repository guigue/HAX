/*
 * h_sntp.c
 *
 * Created: 10/09/2019 14:04:24
 *  Author: User
 */ 

#include <asf.h>
#include <math.h>
#include <inttypes.h>	// Used for PRIu64 

#include "conf_board.h"

#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "sockets.h"
#include "husec.h"
#include "h_sntp.h"

stNTP_PACKET stNTP_packet_tx = { 0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
stNTP_PACKET stNTP_packet_rx = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Timestamp do HICS
// Contador de segundos e milesegundos deste 1970
// Unix epoch: 0x00000000 = 01-01-70 00:00:00 
// Sincronizado periodicamente via SNTP 
// e incrementado a cada 1ms via interrupcao gerada pelo sinal DRDY do AD7770
HICS_TS hts;

void SNTP_Task(void *pvParameters)
//void SNTP_task(void)
{
	UNUSED(pvParameters);

	int32_t lReturned=0;

	// HICS TBD: SE uC RESETAR, O TIMESTAMP VOLTA PARA ZERO (analisar se deve manter o ultimo valor) 
	hts.ulSec		= 0;
	hts.usMs		= 0;
	hts.u64husec	= 0;
	

	vTaskDelay(  (10000UL / portTICK_PERIOD_MS) );	// Delay 10s para aguardar o sistema se estabilizar (1Kms)
	
	while (1)
	{
		
		// Verifica se pode criar socket
		if( stSocket_SNTP.can_create != pdFALSE )
		{	if (vSocket_SNTP_create() == pdPASS)
			{	stSocket_SNTP.can_create = pdFALSE;
			}
		}

		if (stSocket_SNTP.up == pdTRUE)
		{
			
			//vTaskDelay(  (1UL / portTICK_PERIOD_MS) );

			vSocket_SNTP_sendto(&stNTP_packet_tx,sizeof(stNTP_packet_tx));

			/* Receive into the buffer with ulFlags set to 0, so the FREERTOS_ZERO_COPY bit is clear. */
			lReturned = lSocket_SNTP_recv(&stNTP_packet_rx,sizeof(stNTP_packet_rx));

			if( lReturned > 0 )
			{
				/* Data was received from the socket.  Prepare the IP address for
				printing to the console by converting it to a string. */
				//FreeRTOS_inet_ntoa( xSourceAddress.sin_addr, ( char * ) cIPAddressString );

				/* Print out details of the data source. */
				//printf( "\n\nSNTP:"); 

				//printf( “Received %d bytes from IP address %s port number %drn”,
							//iReturned, /* The number of bytes received. */
							//cIPAddressString, /* The IP address that sent the data. */
							//FreeRTOS_ntohs( xSourceAddress.sin_port ) ); /* The source port. */
						
						
				// The fractional part represents the number of fractional units (a unit is 1/((2^32)-1)) in the second.
				// Int32 milliseconds = (Int32)(((Double)fraction / 2^32-1) * 1000);
				volatile uint32_t ulSeconds;		// Utilizando volatile p nao deixar compilador otimizar seq abaixo
				volatile uint32_t ulMilliseconds; 

				volatile double fraction;

				volatile uint16_t ushtsMs;
				volatile uint32_t ulhtsSec; 
				volatile uint8_t  bySync=0;
			
				double fhts, fsts, fdiff;	

				volatile uint64_t u64husec;
			
				fraction = (double) FreeRTOS_ntohl(stNTP_packet_rx.transmit_timestamp_frac); // take care of the endianness
				fraction = (double) fraction / NTP_MAX_INT_AS_DOUBLE;
				fraction = (double) fraction * 1000.0; 

				ulMilliseconds = (uint32_t) fraction;
				if (ulMilliseconds>=1000)	// Se 1s (ou maior) entao tem que descartar
					ulMilliseconds=0;

				// Extrai segundos do pacote NTP
				ulSeconds = FreeRTOS_ntohl(stNTP_packet_rx.transmit_timestamp_sec); // take care of the endianness

				// NTP trabalha com epoch time de 1900 (quantidade de segundos desde 1900)
				// Traz o epoch time para 1970 (unix epoch time)
				ulSeconds = ulSeconds - NTP_TIMESTAMP_DELTA;

				// Obtem timestamp atual do HICS para comparacao e print 
				taskENTER_CRITICAL();
				ushtsMs  = hts.usMs;
				ulhtsSec = hts.ulSec;
				taskEXIT_CRITICAL();

				// Verifica se deve atualizar timestamp do HICS
				//if (ulSeconds > NTP_TIMESTAMP_DELTA )	// Apenas p checar se ha alguma consistencia do valor recebido
				//{
					// Se diferença em milesegundos é maior que +-100 atualiza
					fhts = (double)((double)ulhtsSec  + (double)(((double)ushtsMs)/1000.0));
					fsts = (double)((double)ulSeconds + (double)(((double)ulMilliseconds)/1000.0));
				
					fdiff = fhts - fsts;
					if      ( fdiff > 0.100 )	{bySync=1;} 
					else if ( fdiff < -0.100 )	{bySync=1;} 
				
				//}

				// Calcula HUSEC do pacote NTP
				u64husec = HUSEC_CalcFromNTP();

				if (bySync==1)
				{	
					// ATENCAO, TEM QUE SER ATOMICO
					taskENTER_CRITICAL();	
					hts.usMs     = (uint16_t) ulMilliseconds; 
					hts.ulSec    = ulSeconds; 
					hts.u64husec = u64husec;	
					taskEXIT_CRITICAL();
					// FIM DO ATENCAO
		
				}
			
				// ATENCAO: printf apenas apos sincronizar timers senao vai sincronizar com atraso			
				printf("\nTs:\n");
				printf("Hs=%d ms=%d\n",ulhtsSec,ushtsMs);				// Timestamp HICS antes de atualizar
				printf("Ns=%d ms=%d\n",ulSeconds,ulMilliseconds);		// Timestamp NTP
				printf("Nh=%" PRIu64 "\n",u64husec);

				// FreeRTOS version of printf doesn’t support them print float 
				// (https://www.freertos.org/FreeRTOS_Support_Forum_Archive/April_2018/freertos_How_to_make_printf_sprintf_strtod_thread_safe_44d2ceb8j.html) 
				int8_t ___buffer[50];

				sprintf(___buffer, "%.3f", fdiff); // Utiliza sprintf como workaround		
				___buffer[49]=0;	// Garante NULL
				printf("Diff=%ss\n",___buffer);

				printf("Sync=");
				if (bySync==1) { printf("Y\n"); }
				else { printf("N\n"); }
					
				//vTaskDelay(  (1UL / portTICK_PERIOD_MS) );
	
					
			}

			// HICS:  Sincroniza timer a cada <TBD>
			vTaskDelay(  (60000UL / portTICK_PERIOD_MS) );	// Delay 60s (HICS TBD)
			
		}
	}

}
