#ifndef DATA_TRANSFER_H
#define DATA_TRANSFER_H

#define HOMS_DATA_TRANSFER_VERSION  "20220225T1125BRT"

//---------------------------------------------------------------------------------------------------------
//
//     N E T W O R K      D E F I N T I O N S
//
//---------------------------------------------------------------------------------------------------------
//

#define PORT            15001                        // Defined in the firmware
#define CLIENT_ALLOWED "192.168.1.201"               // HICS IP
#define TCP_SERVER     "homs"
#define IP_SERVER      "192.168.1.102"               // HOMS IP
#define DATA_DIR       "/homs/HATS/data"
#define SDTDPID        "/homs/HATS/log/HOMS_data_transfer_daemon.pid"
#define SDTDBIN        "/opt/HAX/bin/HOMS_data_transfer_daemon"


#define IP_LEN 13
#define TRANSFER_DELAY 100               /* delay between succesive sendings */  
#define WAITING_DELAY  5L                /* delay for waiting the flag ON  */ 
#define SOCKET_DELAY   5000              /* delay in case the connection
					    is broken.  The client will close
 					    the socket wait SOCKET_DELAY 
					    milliseconds, and open a new one */

/*--------------------------oOo-------------------------------------------------------------------------
  
  logging system definitions

  ----------------------------oOo---------------------------------------------------------------------*/
#define  LOG_SHMEM   LOG_LOCAL0          // log file for shared memory
#define  LOG_OTHER   LOG_LOCAL1          // log file for "other" stuff 
#define  LOG_TCPIP   LOG_LOCAL2          // log file for TCP/IP messages
#define  LOG_FILES   LOG_LOCAL3          // log file for files operations
#define  LOG_ACQ     LOG_LOCAL4          // log file for acquiring prog
#define  LOG_CTRL    LOG_LOCAL5          // log file for control
#define  LOG_EPHEM   LOG_LOCAL6          // log file for ephemeris

#define  EMERGENCY   LOG_ALERT           // System unusable. The message will be 
                                         // send to all the consoles.
#define  ALERT       LOG_WARNING         // Message will be showed in one special
                                         // terminal (still to be done).
#define  NOTICE      LOG_NOTICE          // Message will be saved.

/*---------------------------------------------------------------------------------------------------------

     D A T A     D E F I N I T I O N S

     Data structures are prepared for version 1.10 (2 ADC channels only) and 1.20 (5 ADC channels)
     IT MUST BE DEFINED FIRMWARE_VERSION_1.x0 TO COMPILE

     @guiguesp - 2021-10-25 

---------------------------------------------------------------------------------------------------------*/

#define ADC_1CH_BUFFER_SIZEOF 4  	            // header 8-bit + adc 24-bit)


/* ------------------- oOo -----------------------------------------------------
   
   Version 1.10 has 2 ADC channels only (original from Marcio)

   --------------------------oOo----------------------------------------------*/
#ifdef FIRMWARE_VERSION_110

typedef struct {
  unsigned int ulSample;
  unsigned int ulTimestamp_sec;  // Timestamp segundos
  unsigned short usTimestamp_ms; // Timestamp milesegundos
  unsigned long u64husec;	 // Timestamp HUSECS
  // ADC channels raw data
  unsigned int  rawGolay;        // Celula de Golay
  unsigned int  rawPS;           // Power Supply (monitoramento da Tensao de Alimentacao)		
} receive_data_str;
#define DATA_QUARK_SIZE 26
#define ACQ_SHMEM_SIZE 27000    // a little bit more than one second data
#define MAX_BYTES_TO_BLOCK 26000
#endif


/* ------------------- oOo -----------------------------------------------------
   
   Version 1.20 has 5 ADC channels  (from Guigue)

   --------------------------oOo----------------------------------------------*/

#ifdef FIRMWARE_VERSION_120
typedef struct
{
  uint32_t ulSample;
  uint32_t ulTimestamp_sec;	// Timestamp segundos
  uint16_t usTimestamp_ms;	// Timestamp milesegundos
  uint64_t u64husec;		// Timestamp HUSECS
// ADC channels raw data
  uint8_t  rawGolay[ADC_1CH_BUFFER_SIZEOF];	// Celula de Golay
  uint8_t  rawChopper[ADC_1CH_BUFFER_SIZEOF];	// Power Supply (monitoramento da Tensao de Alimentacao)
  uint8_t  rawTemperature_1[ADC_1CH_BUFFER_SIZEOF];	// Temperature on Ch 4
  uint8_t  rawTemperature_2[ADC_1CH_BUFFER_SIZEOF];	// Temperature on Ch 5
  uint8_t  rawTemperature_3[ADC_1CH_BUFFER_SIZEOF];	// Temperature on Ch 6
} receive_data_str;
#define DATA_QUARK_SIZE 38
#define ACQ_SHMEM_SIZE 57000       // 1.5 seconds data
#define MAX_BYTES_TO_BLOCK 38000   // 1 second data
#endif


typedef struct  {
  unsigned char * shmem_ptr ;  
  unsigned long int nrec   ;
} ring_buffer_shmem_info  ;

typedef struct  {
    unsigned int  sample       ;
    unsigned long husec        ;   // HUSECS
    unsigned int  adcuGol      ;   // Golay Cell ADCu
    unsigned int  adcuPS       ;   // Power Supply
} HICS_DATA_STR ;

#define N_TRANS_REC 1000
#define NREC 10000
#define PRECAL_BIN_FP "/opt/HAX/Control/Devices/Hardware/HICS/src/HOMS_precalibration"
#define PRECAL_BIN    "HOMS_precalibration"

// Shared memory definitions
#define ACQ_SHMEM_BCKFILE "acquire_shmem"
#define ACQ_SHMEM_INFO_BCKFILE "acquire_shmem_info"
#define ACQ_SHMEM_PERM      0664
#define MAX_RECORDS_TO_BLOCK 1000
#define ACQ_SEM_NAME        "/acquire_semaphore"
#define ONL_SEM_NAME        "/acq_onl_semaphore"
#define ACQ_SEM_INFO_NAME   "/acquire_semaphore_info"
#define TEN_MIN_MS         600000

#endif
