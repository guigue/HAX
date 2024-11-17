/*
 * husec.c
 *
 * Created: 23/10/2019 10:01:58
 *  Author: User
 */ 

/****************************************************************************************
GUILLERMO GIMENEZ DE CASTRO
Anexos
ter, 15 de out 00:52 (há 3 dias)
para eu, TIAGO

Olá Márcio:

Respeito da base temporal, após algumas consultas e veirificações,
decidi usar o mesmo que estamos usando com outros instrumentos nossos.
Ele é o número de centenas de microsegundos (=0,1 milisegundos) que nós
chamamos de 'husec', *do dia de observação* (não desde 01.01.1970!)

Fiz uma programinha que obtém esse número (variável chamado 'all' do
programa HAXtime.c). A única coisa que deve interessar você é a fórmula
ou procedimento de conversão, que está bastante comentado.

Gostaria que você implementasse este procedimento em seu código e já
fique como o timestamp de nossos dados.

Qualquer problema, conversamos na sexta.

Abraço,

Guigue

HAXtime : Example program to get the time stamp in hundred of microseconds (husec)
          since 0:00:00 UTC using the C time library functions.

          Unix time ("Epoch") does not address correctly the leap seconds and, 
          although for us is a minor problem (har to think we'll observe at 23:59:59)
          I do prefer to use our "traditional" SST solution: the husecs since
          the beginning of the observing day.

          clock_gettime(): gives the time in seconds since 01-01-1970 00:00:00 UTC
          
          To get the number of seconds since 00:00:00 we take the modulus (%)
          of the clock_gettime() output and the number of seconds in a day (86400 
          in the uniform Unix time standard)

          Then we convert this number to husecs and add the nanoseconds (converted 
          to husecs too). 

          To check the results we convert the obtained husecs to hh:mm:ss, and compare
          with the time obatined by using time() function (For some reason, time()
          gives the time in BST and not in UTC).

          This solution will work only while the tv_sec (a signed long int) does not
          overflows. This should happen one day on 2038. Unless the library is
          corrected before.

          Guigue - 14-10-2019 T 18:58 BST

******************************************************************************************/

#include <asf.h>
#include <math.h>
#include <inttypes.h>	// Used for PRIu64 

#include "conf_board.h"
#include "FreeRTOS_IP.h"
#include "h_sntp.h"
#include "husec.h"


uint64_t HUSEC_CalcFromNTP(void)
{
	volatile uint64_t u64husec;

	volatile uint32_t ulSeconds1970;		// Utilizando volatile p nao deixar compilador otimizar seq abaixo
	volatile uint32_t ulNanoseconds;
	volatile uint32_t ulNanoseconds2;

	volatile double fraction;	
	
	// Obtem os nanosegundos do pacote NTP
	// The NTP fractional part represents the number of fractional units (a unit is 1/((2^32)-1)) in the second.
	// Int32 milliseconds = (Int32)(((Double)fraction / 2^32-1) * 1000);
	// Int32 nanoseconds  = (Int32)(((Double)fraction / 2^32-1) * 1000000000);
	
	fraction = (double) FreeRTOS_ntohl(stNTP_packet_rx.transmit_timestamp_frac); // take care of the endianness
	fraction = (double) fraction / NTP_MAX_INT_AS_DOUBLE;
	fraction = (double) fraction * 1000000000.0;

	ulNanoseconds = (uint32_t) fraction;
	if (ulNanoseconds>=1000000000)	// Checa inconsistencia: Se 1s (ou maior) entao tem que descartar
	ulNanoseconds=0;

	// Extrai segundos do pacote NTP
	// NTP trabalha com epoch time de 1900 (quantidade de segundos desde 1900)
	ulSeconds1970 = FreeRTOS_ntohl(stNTP_packet_rx.transmit_timestamp_sec); // take care of the endianness
	ulSeconds1970 = ulSeconds1970 - NTP_TIMESTAMP_DELTA; // Traz o epoch time para 1970 (unix epoch time)

	// HUSEC Convertion Procedure
	u64husec = (uint64_t) (ulSeconds1970 % HUSEC_ONEDAY_SECONDS);	// get the number of seconds of the present day getting
																	// the remainder of the division by 86400 (total number of seconds in a day)

	u64husec = u64husec * HUSEC_SEC2HUSEC;	// convert seconds to husecs

	u64husec = u64husec + (uint64_t) (ulNanoseconds/HUSEC_HUSEC2NSEC);	// convert nanoseconds of the present second to husecs and get the total

	return u64husec;
	
}
