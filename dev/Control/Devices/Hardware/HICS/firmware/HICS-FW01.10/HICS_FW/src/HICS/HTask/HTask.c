/*
 * HTask.c
 *
 * Created: 10/02/2020 17:49:12
 *  Author: User
 */ 

#include <asf.h>
#include <math.h>
#include <inttypes.h>	// Used for PRIu64

#include "conf_board.h"
#include "sockets.h"
#include "h_sntp.h"
#include "husec.h"
#include "adc.h"
#include "h_wdt.h"
#include "h_led.h"
#include "desacopla.h"
#include "htask.h"

HTASK_TICKS_SCHEDULER hsched;

void HICS_Task(void *pvParameters)
{
	UNUSED(pvParameters);

	vTaskDelay(  (5000UL / portTICK_PERIOD_MS) ); // Delay para aguardar o sistema se estabilizar (5000ms)

	// Inicializa tick schedulers
	hsched.ps=0;
	hsched.ps_rst=0;
	hsched.sntp=0;
	hsched.sntp_rst=0;

	// Inicializa ADC e ISR
	ADC_Init();

	WDT_Init();	// Inicializa WDT

	while (wDESAC_enabled == false)
	{
		WDT_Refresh(); // Refresh WDT

		// Verifica se solicitado desacoplamento
		DESAC_Check();
		
		// ADC Task
		ADC_task_steps();
		
		// HICS print status
		if (hsched.ps > HICS_SCHEDULER_PS)
		{
			hsched.ps_rst=1;

			//vTaskDelay(  (1UL / portTICK_PERIOD_MS) );

			printf ("\nErr=%d\n", wHICS_errorcode);
			
			//printf   ("DESAC=%d\n", wDESAC_enabled);

			printf ("FHme=%d\n", xPortGetMinimumEverFreeHeapSize());
			printf ("FH=%d\n", xPortGetFreeHeapSize());

			extern volatile uint16_t wADCcbuff_sizefulln;	// Debug contador buffer full
			extern volatile uint32_t dwADCcbuff_sizefme;	// Debug buffer free size minimum ever
			extern volatile uint32_t dwADCcbuff_sizeact;	// Debug buffer size atual

			printf   ("BFme=%d\n", dwADCcbuff_sizefme);	// Debug buffer free size minimum ever
			printf   ("BFsz=%d\n", dwADCcbuff_sizeact); // Debug buffer size atual
			printf   ("BFn=%d\n", wADCcbuff_sizefulln);	// Debug contador buffer full

			extern volatile uint32_t dwLostSamples_cx;
			extern volatile uint32_t dwLostSamples_old;
			extern volatile uint32_t dwLostSamples_lost_o;
			extern volatile uint32_t dwLostSamples_lost_n;
			printf   ("Lost=%d  %d  %d\n", dwLostSamples_cx, dwLostSamples_lost_o, dwLostSamples_lost_n );
		
		}

		taskYIELD();	// Chaveia tasks

	}
	
	// Cai aqui qdo placas desacopladas
	while(1)	
	{	WDT_Refresh();
		vTaskDelay(  (100UL / portTICK_PERIOD_MS) );	// 100ms	
	}

}

