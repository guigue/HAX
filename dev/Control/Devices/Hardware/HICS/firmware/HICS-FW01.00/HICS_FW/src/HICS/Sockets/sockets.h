/*
 * sockets.h
 *
 * Created: 03/09/2019 14:36:24
 *  Author: User
 */ 


#ifndef SOCKETS_H_
#define SOCKETS_H_

#include "FreeRTOS_Sockets.h"

//-----------------------------------------------------------
// DEFINITIONS
// ----------------------------------------------------------


//-----------------------------------------------------------
// VARS PROTOTYPES
// ----------------------------------------------------------

//extern BaseType_t xCanCreateSockets;
//BaseType_t xADCsocket_up;


typedef struct
{
	Socket_t socket;
	struct freertos_sockaddr addr;
	
	uint8_t can_create;
	uint8_t up;
	
} stSocket;

extern stSocket stSocket_ADC;
extern stSocket stSocket_SNTP;

//-----------------------------------------------------------
// FUNCTIONS PROTOTYPES
// ----------------------------------------------------------

void vSockets_Init(void);

BaseType_t vSocket_ADC_udp_create( void );
void vSocket_ADC_udp_sendto(uint8_t *pucBuffer, uint16_t usTotalDataLength);

BaseType_t vSocket_ADC_tcp_create( void );
BaseType_t  vSocket_ADC_tcp_sendto(uint8_t *pucBuffer, uint16_t usTotalDataLength);
void vSocket_ADC_tcp_close( void );

BaseType_t vSocket_SNTP_create( void );
void vSocket_SNTP_sendto(uint8_t *pucBuffer, uint16_t usTotalDataLength);
int32_t lSocket_SNTP_recv(uint8_t *pucBuffer, uint16_t usTotalDataLength);

#endif /* SOCKETS_H_ */