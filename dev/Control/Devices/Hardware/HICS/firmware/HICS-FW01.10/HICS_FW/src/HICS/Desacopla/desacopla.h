/*
 * desacopla.h
 *
 * Created: 09/04/2020 11:25:43
 *  Author: User
 */ 


#ifndef DESACOPLA_H_
#define DESACOPLA_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

#define DESAC_SWITCH_DEBOUNCE (3000L / portTICK_PERIOD_MS)		// 3s

#define DESAC_SWITCH_OFF	1
#define DESAC_SWITCH_ON		0

#define DESAC_STATE_NOT_ACTIVATE	0
#define DESAC_STATE_ACTIVATE		1


//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

extern uint8_t wDESAC_enabled;

//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

void DESAC_SwicthDebounce(void);
void DESAC_Check(void);
void DESAC_Desacopla(void);


#endif /* DESACOPLA_H_ */