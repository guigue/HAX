/*
 * h_led.h
 *
 * Created: 09/04/2020 10:26:52
 *  Author: User
 */ 


#ifndef H_LED_H_
#define H_LED_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

#define HLED_MODE_NORMAL		0		// Funcionamento normal
#define HLED_MODE_DESACOPLANDO  1		// Placa ADC desacoplando
#define HLED_MODE_DESACOPLADO   2		// Placa ADC desacoplada

#define HLED_TICK_50MS		(50L  / portTICK_PERIOD_MS)		// 50ms
#define HLED_TICK_250MS		(250L / portTICK_PERIOD_MS)		// 250ms
#define HLED_TICK_750MS		(750L / portTICK_PERIOD_MS)		// 750ms

//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

extern uint8_t wHLED_mode;

//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------


void HICS_LED_Toggle(void);

void HICS_LED_IncTick(void);
void HICS_LED_ClearTick(void);

#endif /* H_LED_H_ */