/*


 * HTask.h
 *
 * Created: 10/02/2020 17:49:46
 *  Author: User
 */ 


#ifndef HTASK_H_
#define HTASK_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

#define HICS_TICK_MS		((uint32_t)1UL) // 1ms (definido pela ISR ADC)

#define HICS_SCHEDULER_PS	((uint32_t) (30000UL / HICS_TICK_MS) )	// 30s (HICS TBD)
//#define HICS_SCHEDULER_PS	((uint32_t) (15000UL / HICS_TICK_MS) )	// 15s (HICS TBD)
//#define HICS_SCHEDULER_PS	((uint32_t) (10000UL / HICS_TICK_MS) )	// 10s (HICS TBD)

//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

// HICS ticks scheduler
typedef struct
{	
	uint32_t sntp;	
	uint8_t  sntp_rst;
	
	uint32_t ps;
	uint8_t  ps_rst;

} HTASK_TICKS_SCHEDULER;
extern HTASK_TICKS_SCHEDULER hsched;



//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

void HICS_Task(void *pvParameters);


#endif /* HTASK_H_ */