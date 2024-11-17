/*
 * husec.h
 *
 * Created: 23/10/2019 10:02:16
 *  Author: User
 */ 


#ifndef HUSEC_H_
#define HUSEC_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

#define HUSEC_SEC2HUSEC     ((uint32_t)(10000))		// seconds to husecs
#define HUSEC_HUSEC2NSEC	((uint32_t)(100000))	// husec to nanoseconds 

#define HUSEC_ONEDAY_SECONDS     ((uint32_t)(86400))	// Qtde de segundos em 1 dia

#define HUSEC_ONEDAY_HUSECS   ((uint32_t)(HUSEC_ONEDAY_SECONDS*HUSEC_SEC2HUSEC)) // Qtde de husecs em 1 dia


//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------


//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

uint64_t HUSEC_CalcFromNTP(void);



#endif /* HUSEC_H_ */