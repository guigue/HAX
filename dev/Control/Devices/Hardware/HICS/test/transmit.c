/*
 * 
 * 
 * 
 * 
 * 
 *
 * 
 *
 *
 *
 *
 * Guigue - Jun 2021 - Sao Paulo
 *          May 1998 - Bern
 *          Jun 1998 - Campinas
 */

#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
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
#include "pcdio24.h"
#include "files.h"
#include "buffers.h"

/* function prototypes */
void open_file ( void )                       ;
void ExitOnDemand ( int )                     ; 
int _GetHostName(char *, int )                ;
void save_data ( void )                       ;
void log_msg (unsigned short int, char *, unsigned short int) ;

/* Socket and Communications Parameter definitions */
const int BACK_LOG = 5                        ;
const char ip_allow[] =  CLIENT_ALLOWED       ;
int serverSocket = 0                          ,
    on = 0                                    ,
    port = PORT                               ,
    fp                                        ,
    status = 0                                ;


/* Buffer to receive the data */
char * fname = FILE_SEND_DATA1                ;
char * buffer                                 ;
struct_send_data1 recv_data1                  ;
int sndsize, tamano;
char mmsg[80];

int main()
{
  struct rlimit resourceLimit   = { 0 }     ;
  struct hostent *hostPtr       = NULL      ;
  char * hostname               = TCP_SERVER;
  struct sockaddr_in serverName =  { 0 }    ;
  int dm_lngth                  =  sizeof(struct_send_data1)  ;

#ifdef ITA
  FILE * sdtdpid                            ;
#endif

  int i,status = -1                         ;
  int fileDesc = -1                         ;

  /* Point the buffer to the data structure */
  buffer = (char *) &recv_data1             ;

