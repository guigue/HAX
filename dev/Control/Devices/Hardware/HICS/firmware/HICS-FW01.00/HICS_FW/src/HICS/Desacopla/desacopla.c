/*
 * desacopla.c
 *
 * Created: 09/04/2020 11:25:23
 *  Author: User
 */ 

/* Procedimento para desacoplar:
1. Led piscando normal (250ms)
2. Manter switch 0 pressionado por mais que 3 segundos ate led mudar padrao de piscar
3. Se led comecar a piscar lentamente (750ms) aguardar (placa esta desacoplando - desligamento suave - fechando socket)
4. Quando led comecar a piscar a cada 50ms (rapido) as placas podem ser desligadas
*/


#include <asf.h>
#include "conf_board.h"

#include "adc.h"
#include "h_led.h"
#include "sockets.h"
#include "desacopla.h"

uint8_t  wDESAC_swst  = DESAC_SWITCH_OFF;	// Switch state
uint16_t wDESAC_swdeb = 0;					// Switch debounce

uint8_t wDESAC_enabled = false;

void DESAC_SwicthDebounce(void)
{
	if (wDESAC_swst==DESAC_SWITCH_OFF)
	{
		if (ioport_get_pin_level(GPIO_PUSH_BUTTON_1) == DESAC_SWITCH_ON)	
		{	if (wDESAC_swdeb++ >= DESAC_SWITCH_DEBOUNCE)
			{	wDESAC_swdeb=0;
				wDESAC_swst=DESAC_SWITCH_ON;
				wHLED_mode		= HLED_MODE_DESACOPLANDO;
			}
		}
		else
		{	wDESAC_swdeb=0;	
		}
	}
	else
	{
		if (ioport_get_pin_level(GPIO_PUSH_BUTTON_1) == DESAC_SWITCH_OFF)
		{	if (wDESAC_swdeb++ >= DESAC_SWITCH_DEBOUNCE)
			{	wDESAC_swdeb=0;
			}
		}
		else
		{	wDESAC_swdeb=0;
		}
	}
}


void DESAC_Check(void)
{
	if (wDESAC_swst == DESAC_SWITCH_ON)
	{
		DESAC_Desacopla();
	}
}

#define ioport_set_pin_input_mode(pin, mode, sense) \
do {\
	ioport_set_pin_dir(pin, IOPORT_DIR_INPUT);\
	ioport_set_pin_mode(pin, mode);\
	ioport_set_pin_sense_mode(pin, sense);\
} while (0)



void DESAC_Desacopla(void)
{
	wDESAC_enabled	= true;
	wHLED_mode		= HLED_MODE_DESACOPLADO;
	
	// Close socket
	vSocket_ADC_tcp_close();

	// Desabilita ISR ADC
	NVIC_DisableIRQ(PIOD_IRQn);		// Conector EXT3
	
	/// Reset pin (keep ADC reset)
	ioport_set_pin_level(PIO_PD26_IDX, IOPORT_PIN_LEVEL_LOW);

	// Close SPI (ADC)
	spi_disable(SPI);
	ioport_set_pin_input_mode(SPI_MISO_GPIO,IOPORT_MODE_OPEN_DRAIN,0);
	ioport_set_pin_input_mode(SPI_MOSI_GPIO,IOPORT_MODE_OPEN_DRAIN,0);
	ioport_set_pin_input_mode(SPI_SPCK_GPIO,IOPORT_MODE_OPEN_DRAIN,0);
	ioport_set_pin_input_mode(SPI_NPCS3_PA5_GPIO,IOPORT_MODE_OPEN_DRAIN,0);
	
	printf ("\n\n*** PLACAS uC e ADC DESACOPLADAS ***\n\n");

}

