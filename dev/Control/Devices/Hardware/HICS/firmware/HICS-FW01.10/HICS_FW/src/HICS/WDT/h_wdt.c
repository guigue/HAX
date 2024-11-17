/*
 * h_wdt.c
 *
 * Created: 07/04/2020 11:54:47
 *  Author: User
 */ 

#include <asf.h>
#include "conf_board.h"

#include "desacopla.h"
#include "h_wdt.h"


volatile uint16_t wHICS_errorcode = HICS_ERR_NONE;

volatile uint8_t byWDT_adcisr_ok = 0;
volatile uint8_t byWDT_iptask_ok = 0;

void WDT_Init(void)
{
	uint32_t wdt_mode, timeout_value;

	/* Get timeout value. */
	timeout_value = wdt_get_timeout_value(WDT_PERIOD * 1000, BOARD_FREQ_SLCK_XTAL);
	if (timeout_value == WDT_INVALID_ARGUMENT) {
		while (1) {
			/* Invalid timeout value, error. */
		}
	}
	/* Configure WDT to trigger an interrupt (or reset). */
	wdt_mode =		//WDT_MR_WDFIEN	 |  /* Watchdog Fault Interrupt Enable */
					WDT_MR_WDRSTEN	 |	/* Watchdog Reset Enable */
					//WDT_MR_WDRPROC   |  /* WDT fault resets processor only. */
					WDT_MR_WDIDLEHLT;   /* WDT stops in idle state. */

	/* Initialize WDT with the given parameters. */
	wdt_init(WDT, wdt_mode, timeout_value, timeout_value);
	wdt_get_us_timeout_period(WDT, BOARD_FREQ_SLCK_XTAL);

	/* Configure and enable WDT interrupt. */
	NVIC_DisableIRQ(WDT_IRQn);
	NVIC_ClearPendingIRQ(WDT_IRQn);
	NVIC_SetPriority(WDT_IRQn, 0);
	NVIC_EnableIRQ(WDT_IRQn);
}

void WDT_Refresh(void)
{
	// Faz o refresh do WDT se adc isr e iptask estao rodando ou se hics esta desacoplado da placa adc
	if ( ((byWDT_adcisr_ok==1) && (byWDT_iptask_ok==1)) || (wDESAC_enabled == true) )
	{
		byWDT_adcisr_ok=0;
		byWDT_iptask_ok=0;
	
		/* Clear status bit to acknowledge interrupt by dummy read. */
		wdt_get_status(WDT);
		/* Restart the WDT counter. */
		wdt_restart(WDT);
	}
}


void WDT_Debug_printf_state(uint8_t byReset)
{
	printf ("\nHICS debug state:\n");
	printf ("-FHme=%d\n", xPortGetMinimumEverFreeHeapSize());
	printf ("-FH=%d\n", xPortGetFreeHeapSize());
	
	extern volatile uint32_t dwmyMalloc_cx;
	extern volatile uint32_t dwmyFree_cx;
	printf   ("-Mcx=%d\n", dwmyMalloc_cx);
	printf   ("-Fcx=%d\n", dwmyFree_cx);
	printf   ("-Mdi=%d\n", dwmyMalloc_cx-dwmyFree_cx);
	
	if (byReset==1)
	{	printf ("\nHICS wait reset by watchdog ...\n\n");
		while(1);
	}
	
	
}
