/*
 * h_led.c
 *
 * Created: 09/04/2020 10:26:32
 *  Author: User
 */ 

#include <asf.h>
#include "conf_board.h"

#include "h_led.h"

uint8_t  wHLED_mode = HLED_MODE_NORMAL;
uint16_t wHLED_tick=0;

void HICS_LED_Toggle(void)
{	
	HICS_LED_IncTick();

	if (wHLED_mode == HLED_MODE_DESACOPLANDO)
	{
		if (wHLED_tick >= HLED_TICK_750MS)
		{	HICS_LED_ClearTick();
			LED_Toggle(LED0);
		}
	}
	else if (wHLED_mode == HLED_MODE_DESACOPLADO)
	{
		if (wHLED_tick >= HLED_TICK_50MS)
		{	HICS_LED_ClearTick();
			LED_Toggle(LED0);
		}
	}
	else // if (wHLED_mode == HLED_MODE_NORMAL)
	{
		if (wHLED_tick >= HLED_TICK_250MS)
		{	HICS_LED_ClearTick();
			LED_Toggle(LED0);
		}
	}

}

void HICS_LED_IncTick(void)
{
	wHLED_tick++;
}

void HICS_LED_ClearTick(void)
{
	//taskENTER_CRITICAL();
	wHLED_tick=0;
	//taskEXIT_CRITICAL();
}


