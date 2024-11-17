/*
 * h_sntp.h
 *
 * Created: 10/09/2019 14:04:50
 *  Author: User
 */ 


#ifndef H_SNTP_H_
#define H_SNTP_H_

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------

#define NTP_MAX_INT_AS_DOUBLE	((double)(4294967295.0))	// 2^32-1
//#define NTP_MAX_INT_AS_DOUBLE	((float)(4294967295.0))	// 2^32-1

#define NTP_TIMESTAMP_DELTA		((uint32_t)(2208988800))	// Offset em segundos de 01/01/1900 ate 01/01/1970


//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

// Structure that defines the 48 byte NTP packet protocol
typedef struct _NTP_MSG			// NTP v4
{
	uint8_t  li_vn_mode;		// Leap Indicator, Version Number, Mode
	uint8_t  stratum;			// Stratum, or type of clock
	uint8_t  poll;				// Peer polling exponent
	int8_t   precision;			// Peer Clock Precision (signed)
	uint32_t root_delay;		// Root Delay
	uint32_t root_dispersion;	// Root Dispersion
	uint32_t reference_id;		// Reference ID
	uint32_t reference_timestamp_sec;
	uint32_t reference_timestamp_frac;
	uint32_t origin_timestamp_sec;
	uint32_t origin_timestamp_frac;
	uint32_t receive_timestamp_sec;
	uint32_t receive_timestamp_frac;
	uint32_t transmit_timestamp_sec;
	uint32_t transmit_timestamp_frac;
} stNTP_PACKET;
extern stNTP_PACKET stNTP_packet_rx;

// HICS Timestamp
typedef struct
{	uint32_t ulSec;		// Seconds (contador de segundos desde 01-01-70 00:00:00 - Unix epoch)
	uint16_t usMs;		// Mileseconds (0 a 999)
	
	 uint64_t u64husec;	// husecs since the beginning of the observing day
						// número de centenas de microsegundos (=0,1 milisegundos) que nós
						// chamamos de 'husec', *do dia de observação*
} HICS_TS;
extern HICS_TS hts;


//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

//void SNTP_task(void);
void SNTP_Task(void *pvParameters);


#endif /* H_SNTP_H_ */