/*
 * h_wdt.h
 *
 * Created: 07/04/2020 11:55:05
 *  Author: User
 */ 


#ifndef H_WDT_H_
#define H_WDT_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

// HICS CODIGO DE ERROS BINARIOS (OR)
#define HICS_ERR_NONE 			0x0000
#define HICS_ERR_WDT_RST		0x0001
#define HICS_ERR_DESACOPLADO	0x0002
#define HICS_ERR_TBD2			0x0004


/** Watchdog period */
//#define WDT_PERIOD                        300  // 300ms
//#define WDT_PERIOD                        1000  // 1s
#define WDT_PERIOD                        10000  // 10s

//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

extern volatile uint16_t wHICS_errorcode;

extern volatile uint8_t byWDT_adcisr_ok;
extern volatile uint8_t byWDT_iptask_ok;

//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

void WDT_Init(void);

void WDT_Refresh(void);

void WDT_Debug_printf_state(uint8_t byReset);

#endif /* H_WDT_H_ */
