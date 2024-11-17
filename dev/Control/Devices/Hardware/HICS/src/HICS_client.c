/*********************************************************************
 *
 * HICS_client:
 *     A fake simulator of HICS sending fake data
 *     Adapted from cdt.c of SST. 
 *     @guiguesp - Aug 2021 - Sao Paulo 
 * 
 * -------------------------------------------------------------------
 *
 * CDT: Client Data Transfer
 *      Transfer values from the shared memory struct_send_data1
 *      and send them through TCP/IP to Display Computer (robinson)
 *
 * Note: Must be started on acqusition node (node2) !
 *
 * Guigue - May 1998 - Bern (! feelling like Luifa Scola)
 */

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "data_transfer.h"

/* Structures and variables for the Socket Connection */
int clientSocket      ,
    remotePort = PORT ,
    status     = 0    ;
struct hostent * hostPtr = NULL            ;
struct sockaddr_in socketClient_in         ;
struct in_addr homs_addr                   ;
char * remoteHost = TCP_SERVER             ;

receive_data_str fake_data[NREC]           , // Fake Data
  sent_data[N_TRANS_REC]                   ;
char * buffer                              ; // pointer to the data
int size_of_data = sizeof(sent_data)       ; // size of the data structure

void ExitCleanly ( )            ;
void ExitOnDemand ( int )       ;
void get_socket ( void )        ;
void open_connection ( void )   ; 
void log_msg( int, char *, int) ;
void fill_in_data( void )       ;
void delay( int )               ;

int main()
{
  int istep          ; 
  fill_in_data( )    ;
  get_socket( )      ;
  open_connection()  ;
  for (istep=0;istep<N_TRANS_REC/NREC;istep++){
    
    buffer = (char *) fake_data[istep*N_TRANS_REC];
    if(size_of_data > (status = send(clientSocket, buffer, size_of_data, MSG_WAITALL)))
      log_msg(LOG_TCPIP, "Incomplete transmission with server ", ALERT);
    
  }

}

/*************************** END OF MAIN ********************************/


/* 
   ExitCleanly

   For use from an external "killer" as "kill ...."
*/

void ExitCleanly ( ) {
  close(clientSocket)   ;
  log_msg(LOG_TCPIP, "Aborted ", ALERT);
  closelog()            ;
  _exit(0)              ;
}


/*------------------------------------------------- 
   open_connection

   Connect if possible
   if not, retry the connection
---------------------------------------------------*/

void open_connection ( void ) {

  while ( -1 == (status = connect(clientSocket, (struct sockaddr *)&socketClient_in, sizeof(socketClient_in))))
  {
    log_msg( LOG_TCPIP,"cannot open connection with server",ALERT);
	close(clientSocket)   ;
	delay(SOCKET_DELAY)   ;
	get_socket ()         ; 
  }

  log_msg(LOG_TCPIP, "Connection established with Server", ALERT);

  return;

}

/*-------------------------------------------------------------- 

   get_socket

   Get an Internet Socket to connect with display server
   Get the IP number also

----------------------------------------------------------------*/
void get_socket ( void ){

  /* 
     Get a Socket for communication
     Socket will be of type AF_INET (Internet Domain Adress Format)
     Stream, SOCK_STREAM
     TCP/IP protocol, IPPROTO_TCP 

     In general, the while loop should just run once, it is suposed
     the Computer will not have a lot of IPC sockets in use :-)
  */
     
  while (-1 == (clientSocket = socket(AF_INET, SOCK_STREAM, 0)))
    log_msg(LOG_TCPIP, "cannot get a socket to communicate", NOTICE);

  inet_aton(IP_SERVER,&homs_addr);
  //if ( NULL == (hostPtr = gethostbyname(remoteHost)))
  if ( NULL == (hostPtr = gethostbyaddr(&homs_addr, sizeof(homs_addr),AF_INET)))
  {
    log_msg( LOG_TCPIP,"error resolving server address ",NOTICE)  ;

  } else {
    bzero(&socketClient_in,sizeof(socketClient_in));
    socketClient_in.sin_family = AF_INET         ;
    socketClient_in.sin_addr.s_addr = inet_addr(IP_SERVER);
    socketClient_in.sin_port = htons(remotePort)  ;
    log_msg( LOG_TCPIP,"Got socket ",NOTICE)  ;
  }

  return ;

}


void ExitOnDemand ( int sig_no ) {
  log_msg( LOG_TCPIP ,"Finish receiving", NOTICE)  ;
  close(clientSocket)   ;
  closelog ( )          ;
  _exit(0)              ;
}

void fill_in_data( void) {
  
  int i;

  for (i=0;i<NREC;i++) fake_data[i].ulSample= (long) i;

}


void delay( int seconds) {
  
  // Taken from Geeks for Geeks: https://www.geeksforgeeks.org/time-delay-c/
  
   // Converting time into milli_seconds
    int milli_seconds = 1000 * seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds);
}
