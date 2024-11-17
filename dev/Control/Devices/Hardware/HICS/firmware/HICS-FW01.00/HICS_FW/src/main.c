/**
 * \file
 *
 * \brief FreeRTOS configuration
 *
 * Copyright (c) 2012-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source softwinare) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */
/*
 * Support and FAQ: visit <a href="https://www.microchip.com/support/">Microchip Support</a>
 */

 
#include <asf.h>

#include <string.h>		
#include <inttypes.h>	// Used for PRIu64


#include "conf_usb.h"
#include "conf_board.h"
#include "ui.h"

#include "FreeRTOS_IP.h"

#include "FreeRTOS_Sockets.h" // HICS

#include "adc.h"
#include "sockets.h"
#include "h_sntp.h"
#include "htask.h"
#include "h_wdt.h"

static volatile bool main_b_msc_enable = false;

static void main_memories_trans_task(void *pvParameters);

//! Handle to the semaphore protecting memories transfer.
static xSemaphoreHandle main_trans_semphr = NULL;


/**
 *  Configure UART console.
 */
static void configure_console(void)
{
	const usart_serial_options_t uart_serial_options = {
		.baudrate = CONF_UART_BAUDRATE,
#ifdef CONF_UART_CHAR_LENGTH
		.charlength = CONF_UART_CHAR_LENGTH,
#endif
		.paritytype = CONF_UART_PARITY,
#ifdef CONF_UART_STOP_BITS
		.stopbits = CONF_UART_STOP_BITS,
#endif
	};

	/* Configure console UART. */
	sysclk_enable_peripheral_clock(CONSOLE_UART_ID);
	pio_configure_pin_group(CONF_UART_PIO, CONF_PINS_UART,
			CONF_PINS_UART_FLAGS);
	stdio_serial_init(CONF_UART, &uart_serial_options);
}


/* The MAC address array is not declared const as the MAC address will
normally be read from an EEPROM and not hard coded (in real deployed
applications).*/
const uint8_t ucMACAddress[ 6 ] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 }; 

/* Define the network addressing.  These parameters will be used if either
ipconfigUDE_DHCP is 0 or if ipconfigUSE_DHCP is 1 but DHCP auto configuration
failed. */
static const uint8_t ucIPAddress[ 4 ] = { 192, 168, 1, 201 };		// SP
static const uint8_t ucNetMask[ 4 ] = { 255, 255, 255, 0 };
static const uint8_t ucGatewayAddress[ 4 ] = { 192, 168, 1, 1 };	// SP

/* The following is the address of an OpenDNS server. */
static const uint8_t ucDNSServerAddress[ 4 ] = { 208, 67, 222, 222 };

/* Use by the pseudo random number generator. */
static UBaseType_t ulNextRand;

UBaseType_t uxRand( void )
{
	const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

	/* Utility function to generate a pseudo random number. */

	ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
	return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}
// HICS_end

// HICS
BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )	
{
	*( pulNumber ) = uxRand();
}


// HICS 
BaseType_t xApplicationDNSQueryHook( const char *pcName )
{
	BaseType_t xReturn;

	return xReturn;
}

/**
 * \brief Configure the SMC for SRAM access.
 *
 * \param cs  Chip select.
 */
static void configure_sram(uint32_t cs)
{
	smc_set_setup_timing(SMC, cs, SMC_SETUP_NWE_SETUP(1)
			| SMC_SETUP_NCS_WR_SETUP(1)
			| SMC_SETUP_NRD_SETUP(1)
			| SMC_SETUP_NCS_RD_SETUP(1));
	smc_set_pulse_timing(SMC, cs, SMC_PULSE_NWE_PULSE(6)
			| SMC_PULSE_NCS_WR_PULSE(6)
			| SMC_PULSE_NRD_PULSE(6)
			| SMC_PULSE_NCS_RD_PULSE(6));
	smc_set_cycle_timing(SMC, cs, SMC_CYCLE_NWE_CYCLE(7)
			| SMC_CYCLE_NRD_CYCLE(7));
	smc_set_mode(SMC, cs, SMC_MODE_READ_MODE | SMC_MODE_WRITE_MODE);
}


// HICS check if sprintf is ok
volatile int8_t		buffer6[50];	
volatile uint32_t	u1=27;	
volatile float		f1=3.1415;	
volatile double		d1=6.19;	


/*! \brief Main function. Execution starts here.
 */
int main(void)
{
	irq_initialize_vectors();
	cpu_irq_enable();

	// Initialize the sleep manager
	sleepmgr_init();
	sysclk_init();

	board_init();
	ui_init();
	//ui_powerdown();


	/* Enable PMC clock for SMC */
	pmc_enable_periph_clk(ID_SMC);
	/* SMC configuration between SRAM and SMC waveforms. */
	configure_sram(SRAM_CHIP_SELECT);		// SRAM1: HEAP			(EBI_CS1 @0x61000000)
	configure_sram(SRAM_CHIP_SELECT_2ND);	// SRAM2: BUFFER ADC	(EBI_CS3 @0x63000000)

	/* Initialize the console uart */
	configure_console();

	// HICS
	printf ("\n\n");
	printf ("*** CRAAM - Centro de Radio-Astronomia e Astrofisica Mackenzie          ***\n");
	printf ("*** HATS::HICS - Data Acquisition Module                                ***\n");
	printf ("*** Firmware release 1.00 - 22/June/2020                                ***\n\n");
	
	
	printf("-- %s\r\n", BOARD_NAME);
	printf("-- Compiled: %s %s --\r\n\r\n", __DATE__, __TIME__);
	// HICS_end

	printf ("\n\nHICS-ErrorCode=0x%04X\n\n", wHICS_errorcode);
	
	ctrl_access_init(); // Required with FreeRTOS
	//memories_initialization();

	// HICS debug
	printf ("MinimumEverFreeHeapSize = %d\n", xPortGetMinimumEverFreeHeapSize());
	printf ("FreeHeapSize            = %d\n", xPortGetFreeHeapSize());
	// HICS_end


	// HICS: Check if sprintf is ok
	printf("\nsprintf test:\n");
	sprintf(buffer6, "%d", u1); // Utiliza sprintf como workaround
	buffer6[49]=0;	// Garante NULL
	printf("u1=%ss\n",buffer6);
	
	sprintf(buffer6, "%.3f", f1); // Utiliza sprintf como workaround
	buffer6[49]=0;	// Garante NULL
	printf("f1=%ss\n",buffer6);

	sprintf(buffer6, "%.3f", d1); // Utiliza sprintf como workaround
	buffer6[49]=0;	// Garante NULL
	printf("d1=%ss\n\n",buffer6);
	

	// HICS
	printf("Sizeof buffer ADC:\n" ); 
	printf("	=> Tamanho de um registro : %d bytes\n",sizeof(CHBUF_DATA)); 
	printf("	=> Quantidade de registros: %d\n",ADC_BUFFER_LINES);
	printf("	=> Tamanho do Buffer:       %d bytes\n\n",sizeof(chcbuf_data));

	// HICS: seguindo procedimento contido em gmac.h (ver tbem prvGMACInit())
    pmc_enable_periph_clk(ID_GMAC);

	// Antes de criar tasks e sockets, sinaliza que sockets nao estao criados
	vSockets_Init();
	
	/* Initialise the RTOS's TCP/IP stack.  The tasks that use the network
    are created in the vApplicationIPNetworkEventHook() hook function
    below.  The hook function is called when the network connects. */
    FreeRTOS_IPInit( ucIPAddress,
                     ucNetMask,
                     ucGatewayAddress,
                     ucDNSServerAddress,
                     ucMACAddress );


	sleepmgr_lock_mode(SLEEPMGR_ACTIVE);	// HICS: Nao pode deixar entrar em sleep
											

	// Main Task (scheduler)
	xTaskCreate(HICS_Task,((const signed portCHAR *)"HICS_Task"),	4096, NULL,configMAX_PRIORITIES-2, NULL);	
	xTaskCreate(SNTP_Task,((const signed portCHAR *)"SNTP-TASK"),	1024, NULL, configMAX_PRIORITIES-2, NULL);	

	// Start OS scheduler
	vTaskStartScheduler();
	
	return 0;
}

/* Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
events are only received if implemented in the MAC driver. */
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
char cBuffer[ 16 ];
static BaseType_t xTasksAlreadyCreated = pdFALSE;

	/* If the network has just come up...*/
	if( eNetworkEvent == eNetworkUp )
	{
		/* Print out the network configuration, which may have come from a DHCP
		server. */
		FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
		FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
		FreeRTOS_printf( ( "\r\n\r\nIP Address: %s\r\n", cBuffer ) );

		FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
		FreeRTOS_printf( ( "Subnet Mask: %s\r\n", cBuffer ) );

		FreeRTOS_inet_ntoa( ulGatewayAddress, cBuffer );
		FreeRTOS_printf( ( "Gateway Address: %s\r\n", cBuffer ) );

		FreeRTOS_inet_ntoa( ulDNSServerAddress, cBuffer );
		FreeRTOS_printf( ( "DNS Server Address: %s\r\n\r\n", cBuffer ) );
	
		
		/* Create the tasks that use the IP stack if they have not already been
		created. */
		if( xTasksAlreadyCreated == pdFALSE )
		{
			/* See the comments above the definitions of these pre-processor
			macros at the top of this file for a description of the individual
			demo tasks. */
			#if( mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS == 1 )
			{
				vStartSimpleUDPClientServerTasks( configMINIMAL_STACK_SIZE, mainSIMPLE_UDP_CLIENT_SERVER_PORT, mainSIMPLE_UDP_CLIENT_SERVER_TASK_PRIORITY );
			}
			#endif /* mainCREATE_SIMPLE_UDP_CLIENT_SERVER_TASKS */

			#if( mainCREATE_TCP_ECHO_TASKS_SINGLE == 1 )
			{
				vStartTCPEchoClientTasks_SingleTasks( mainECHO_CLIENT_TASK_STACK_SIZE, mainECHO_CLIENT_TASK_PRIORITY );
			}
			#endif /* mainCREATE_TCP_ECHO_TASKS_SINGLE */

			#if( mainCREATE_TCP_ECHO_SERVER_TASK == 1 )
			{
				vStartSimpleTCPServerTasks( mainECHO_SERVER_TASK_STACK_SIZE, mainECHO_SERVER_TASK_PRIORITY );
			}
			#endif

			// HICS: Sinaliza que sockets ja podem ser criados
			// From: https://www.freertos.org/FreeRTOS_Support_Forum_Archive/April_2018/freertos_Need_help_with_example_project_Using_the_FreeRTOS_Windows_Port_ed0a9982j.html
			//		=> vApplicationIPNetworkEventHook() is being called from the IP-task, and it is not allowed to call FreeRTOS_bind() from within the IP-task.
			//		=> The best solution is to keep vApplicationIPNetworkEventHook() very short, just set a variable:
			 stSocket_ADC.can_create  = pdTRUE;
 			 stSocket_SNTP.can_create = pdTRUE;
			// HICS_end

			xTasksAlreadyCreated = pdTRUE;
		}
	}
}

void vApplicationIdleHook( void );
void vApplicationIdleHook( void )
{
	// Management of sleep mode in Idle Hook from FreeRTOS
	// HICS removed: sleepmgr_enter_sleep();
	// https://www.freertos.org/a00016.html?_ga=2.97969567.1806821866.1583097460-1965068279.1581949294
	
}

static void main_memories_trans_task(void *pvParameters)
{
	UNUSED(pvParameters);
	while (true) {
		// Wait for a semaphore which signals that a transfer is requested
		if( xSemaphoreTake( main_trans_semphr, portMAX_DELAY ) == pdTRUE ) {
			udi_msc_process_trans();
		}
	}
}

void main_msc_notify_trans(void)
{
	static signed portBASE_TYPE xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	// One transfer is requested 
	// It is now time for main_memories_trans_task() to run
	xSemaphoreGiveFromISR( main_trans_semphr, &xHigherPriorityTaskWoken );
}

void main_suspend_action(void)
{
	ui_powerdown();
}

void main_resume_action(void)
{
	ui_wakeup();
}

void main_sof_action(void)
{
	if (!main_b_msc_enable)
		return;
	ui_process(udd_get_frame_number());
}

bool main_msc_enable(void)
{
	main_b_msc_enable = true;
	return true;
}

void main_msc_disable(void)
{
	main_b_msc_enable = false;
}

/**
 * \mainpage ASF USB Device MSC
 *
 * \section intro Introduction
 * This example shows how to implement a USB Device Mass Storage
 * on Atmel MCU products with USB module.
 *
 * \section startup Startup
 * The example uses all memories available on the board and connects these to
 * USB Device Mass Storage stack. After loading firmware, connect hardware board
 * (EVKxx,Xplain,...) to the USB Host. When connected to a USB host system
 * this application allows to display all available memories as a
 * removable disks in the Unix/Mac/Windows operating systems.
 * \note
 * This example uses the native MSC driver on Unix/Mac/Windows OS, except for Win98.
 *
 * \copydoc UI
 *
 * \section example About example
 *
 * The example uses the following module groups:
 * - Basic modules:
 *   Startup, board, clock, interrupt, power management
 * - USB Device stack and MSC modules:
 *   <br>services/usb/
 *   <br>services/usb/udc/
 *   <br>services/usb/class/msc/
 * - Specific implementation:
 *    - main.c,
 *      <br>initializes clock
 *      <br>initializes interrupt
 *      <br>manages UI
 *    - specific implementation for each target "./examples/product_board/":
 *       - conf_foo.h   configuration of each module
 *       - ui.c        implement of user's interface (leds)
 *
 * \subsection freertos FreeRTOS
 * \paragraph tasks Task required
 * The USB device stack is fully interrupt driven and uses the USB DMA to transfer
 * data from/to the internal RAM. Thus, the USB does not require task creation.
 * However, the data memory transfer is done without DMA and requires a specific task
 * (called main_memories_trans_task).
 * 
 * \paragraph sync Task synchronization
 * The memory transfer task is synchronized (wakeup) by a mutex
 * (main_trans_semphr) which is given by a USB notification callback
 * (main_msc_notify_trans()). The callback is called by the USB interrupt which
 * is FreeRTOS compliant (See ISR_FREERTOS() macro).
 *
 * \paragraph power Sleep mode in FreeRTOS
 * The USB stack supports the sleepmgr module which allows to easily use the
 * CPU sleep mode and reduce the power consumption. To use sleepmgr modules 
 * in FreeRTOS, the best option is to enable configUSE_IDLE_HOOK and define
 * vApplicationIdleHook() which calls sleepmgr_enter_sleep().
 *
 * However, the IDLE sleep level must be locked in sleepmgr module, if you want
 * that FreeRTOS runs when USB cable is unplugged. In fact, the Timer counter
 * used by FreeRTOS runs only when sleep level is > IDLE.
 */
