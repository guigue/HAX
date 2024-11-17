/*
 * sockets.c
 *
 * Created: 03/09/2019 14:36:02
 *  Author: User
 */ 

#include <asf.h>
#include "conf_board.h"

#include "FreeRTOSIPConfig.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "sockets.h"

// Reference: https://www.freertos.org/FreeRTOS-Plus/FreeRTOS_Plus_TCP/TCP_Networking_Tutorial_TCP_Client_and_Server.html

static const char     SOCKET_ADC_UDP_IP[]	= { "192.168.1.102" };	// My notebook IP (SP)
static const uint16_t SOCKET_ADC_UDP_PORT   = 15000;				// My notebook PORT

static const char     SOCKET_ADC_TCP_IP[]	= { "192.168.1.102" };	// My notebook IP (SP)
static const uint16_t SOCKET_ADC_TCP_PORT   = 15001;				// My notebook PORT


static const char     SOCKET_SNTP_IP[]	= { "192.168.1.102" };	// My notebook IP (SP)
static const uint16_t SOCKET_SNTP_PORT  = 123;					// SNTP port

stSocket stSocket_ADC;

stSocket stSocket_SNTP;


void vSockets_Init(void)
{
	// ATENCAO: Antes de criar tasks e sockets, sinaliza que sockets nao estao criados

	// ADC
	stSocket_ADC.can_create = pdFALSE;
	stSocket_ADC.up         = pdFALSE;

	// SNTP	
	stSocket_SNTP.can_create = pdFALSE;
	stSocket_SNTP.up         = pdFALSE;

}

/* ----------------------------------------------------------------------------------------- */

BaseType_t vSocket_ADC_udp_create( void )
{
	// Set IP::PORT
	stSocket_ADC.addr.sin_addr = FreeRTOS_inet_addr( &SOCKET_ADC_UDP_IP );
	stSocket_ADC.addr.sin_port = FreeRTOS_htons( SOCKET_ADC_UDP_PORT );

	/* Create the socket. */
	stSocket_ADC.socket = FreeRTOS_socket( FREERTOS_AF_INET,
								  FREERTOS_SOCK_DGRAM,/*FREERTOS_SOCK_DGRAM for UDP.*/
                                  FREERTOS_IPPROTO_UDP );

	/* Check the socket was created. */
	//configASSERT( stSocket_ADC.socket != FREERTOS_INVALID_SOCKET );

	if (stSocket_ADC.socket != FREERTOS_INVALID_SOCKET)
	{	stSocket_ADC.up = pdTRUE;	// Sinaliza socket ok para enviar pacotes
		printf ("\nUDP-Socket-ADC is up!\n\n");
		return (pdPASS);
	}
	else
	{	
		printf("\n\nERROR: UDP-Socket-ADC is not created!\n\n");
		return (pdFAIL);
	}

   /* NOTE: FreeRTOS_bind() is not called.  This will only work if
   ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 in FreeRTOSIPConfig.h. */
  

}

void vSocket_ADC_udp_sendto(uint8_t *pucBuffer, uint16_t usTotalDataLength)
{ 

    FreeRTOS_sendto( stSocket_ADC.socket,
					pucBuffer,
					usTotalDataLength,
					0,
					&stSocket_ADC.addr,
					sizeof( stSocket_ADC.addr ) );
}


/* ----------------------------------------------------------------------------------------- */

BaseType_t vSocket_ADC_tcp_create( void )
{
	// Set IP::PORT
	stSocket_ADC.addr.sin_addr = FreeRTOS_inet_addr( &SOCKET_ADC_TCP_IP );
	stSocket_ADC.addr.sin_port = FreeRTOS_htons( SOCKET_ADC_TCP_PORT );

	/* Create the socket. */
	stSocket_ADC.socket = FreeRTOS_socket( FREERTOS_AF_INET,
								  FREERTOS_SOCK_STREAM,/* FREERTOS_SOCK_STREAM for TCP. */
                                  FREERTOS_IPPROTO_TCP  );

	/* Check the socket was created. */
	configASSERT( stSocket_ADC.socket != FREERTOS_INVALID_SOCKET );

    /* Connect to the remote socket.  The socket has not previously been bound to
    a local port number so will get automatically bound to a local port inside
    the FreeRTOS_connect() function. */
    if( FreeRTOS_connect( stSocket_ADC.socket, &stSocket_ADC.addr, sizeof( stSocket_ADC.addr ) ) == 0 )
	{	stSocket_ADC.up = pdTRUE;	// Sinaliza socket ok para enviar pacotes
		printf ("\nTCP-Socket-ADC is up!\n\n");
		return (pdPASS);
	}
	else
	{	
		printf("\n\nERROR: TCP-Socket-ADC is not created!\n\n");
		return (pdFAIL);
	}


}


BaseType_t  vSocket_ADC_tcp_sendto(uint8_t *pucBuffer, uint16_t usTotalDataLength)
{ 
    return FreeRTOS_send(stSocket_ADC.socket,
					pucBuffer,
					usTotalDataLength,
					0 ); /* ulFlags. */
}


void vSocket_ADC_tcp_close( void )
{
	uint8_t cRxedData[ 64 ];
	uint8_t byTimeout=0;
	
	/* Initiate graceful shutdown. */
    FreeRTOS_shutdown( stSocket_ADC.socket, FREERTOS_SHUT_RDWR );

    /* Wait for the socket to disconnect gracefully (indicated by FreeRTOS_recv()
    returning a FREERTOS_EINVAL error) before closing the socket. */
    while(byTimeout++ <= 4) // Timeout 1s
	{
		if (FreeRTOS_recv( stSocket_ADC.socket, &cRxedData, sizeof(cRxedData), 0 ) <  0 )
			break;
			
        /* Wait for shutdown to complete.  If a receive block time is used then
        this delay will not be necessary as FreeRTOS_recv() will place the RTOS task
        into the Blocked state anyway. */
        //vTaskDelay( pdTICKS_TO_MS( 250 ) );
		vTaskDelay(  (250 / portTICK_PERIOD_MS) );	// Delay 250ms

        /* Note – real applications should implement a timeout here, not just
        loop forever. */ /*** DONE ***/
    }

    /* The socket has shut down and is safe to close. */
    FreeRTOS_closesocket( stSocket_ADC.socket );
	
	printf ("\nTCP-Socket-ADC is down!\n\n");
	
}



/* ----------------------------------------------------------------------------------------- */

BaseType_t vSocket_SNTP_create( void )
{
	// Set IP::PORT
	stSocket_SNTP.addr.sin_addr = FreeRTOS_inet_addr( &SOCKET_SNTP_IP );
	stSocket_SNTP.addr.sin_port = FreeRTOS_htons( SOCKET_SNTP_PORT );

	/* Create the socket. */
	stSocket_SNTP.socket = FreeRTOS_socket( FREERTOS_AF_INET,
								  FREERTOS_SOCK_DGRAM,/*FREERTOS_SOCK_DGRAM for UDP.*/
                                  FREERTOS_IPPROTO_UDP );

	/* Check the socket was created. */
	//configASSERT( stSocket_SNTP.socket != FREERTOS_INVALID_SOCKET );

	if (stSocket_SNTP.socket != FREERTOS_INVALID_SOCKET)
	{	stSocket_SNTP.up = pdTRUE;	// Sinaliza socket ok para enviar pacotes
		printf ("\nSocket-SNTP is up!\n\n");
		return (pdPASS);
	}
	else
	{	
		printf("\n\nERROR: Socket-SNTP is not created!\n\n");
		return (pdFAIL);
	}


   /* NOTE: FreeRTOS_bind() is not called.  This will only work if
   ipconfigALLOW_SOCKET_SEND_WITHOUT_BIND is set to 1 in FreeRTOSIPConfig.h. */

}

void vSocket_SNTP_sendto(uint8_t *pucBuffer, uint16_t usTotalDataLength)
{

	FreeRTOS_sendto( stSocket_SNTP.socket,
	pucBuffer,
	usTotalDataLength,
	0,
	&stSocket_SNTP.addr,
	sizeof( stSocket_SNTP.addr ) );
}


int32_t lSocket_SNTP_recv(uint8_t *pucBuffer, uint16_t usTotalDataLength)
{
	int32_t lReturned;
	
	/* Receive into the buffer with ulFlags set to 0, so the FREERTOS_ZERO_COPY bit is clear. */
	lReturned = FreeRTOS_recvfrom(
                                /* The socket data is being received on. */
                                stSocket_SNTP.socket,
                                /* The buffer into which received data will be
                                copied. */
                                pucBuffer,
                                /* The length of the buffer into which data will be
                                copied. */
                                usTotalDataLength,
                                /* ulFlags with the FREERTOS_ZERO_COPY bit clear. */
                                0,
                                /* Will get set to the source of the received data. */
                                &stSocket_SNTP.addr,
                                /* Not used but should be set as shown. */
                                sizeof( stSocket_SNTP.addr )
                            );

	return lReturned;
	
}

