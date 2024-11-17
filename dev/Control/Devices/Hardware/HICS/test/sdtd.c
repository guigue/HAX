/*
 * 
 * 
 * SDTD: Server Data Transfer Daemon
 *      Transfer values from the shared memory struct_send_data1
 *      and send them through TCP/IP to Display Computer (robinson)
 *
 * Note: Must be started on acqusition IDL machine (twain)!
 *
 *
 *
 *
 * Guigue - May 1998 - Bern
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
#include "atmio16x.h"
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

int main()
{
    struct rlimit resourceLimit   = { 0 }     ;
    struct hostent *hostPtr       = NULL      ;
    char * hostname               = TCP_SERVER;
    struct sockaddr_in serverName =  { 0 }    ;
    int dm_lngth                  =  sizeof(struct_send_data1)  ;
    FILE * sdtdpid                            ;

    int i,status = -1                         ;
    int fileDesc = -1                         ;

    /* Point the buffer to the data structure */
    buffer = (char *) &recv_data1             ;


/**********************************************************************

	  DAEMON DEFINITION 



       Start the operations to convert the program to a Unix Daemon.

       The following procedure was taken from the article:
           "Linux Network Programming, Part 2: Creating Daemon Processes"
	    Ivan Grifith and John Nelson
	    Linux Journal, 47, March 1998

       The first step is to create a new identical process with fork()

       */
       
    status = fork();

    switch (status)
    {
    case -1:
        perror("fork()");  /* Couldn't fork, exit */
        exit(1);

    case 0:                /* child process, will continue living  */
        break;

    default:               /* parent process, will die */
        exit(0);
    }


    /*
        Now, we continue with the child process

        Will close ALL open file descriptors inherited from its parent.
        We do it in a "killer" way, closing ALL the possible file
        descriptors.  To know this number, we use getrlimit() and then
        proceed.

     */

    resourceLimit.rlim_max = 0;

    status = getrlimit(RLIMIT_NOFILE, &resourceLimit);
    if (-1 == status) {
        perror("getrlimit()");
        exit(1);
    }
    if (0 == resourceLimit.rlim_max)
    {
        printf("Max number of open file descriptors is 0!!\n");
        exit(1);
    }
    for (i = 0; i < resourceLimit.rlim_max; i++)
      (void) close(i);

    /* 

      Change the process group.

      I copy Grifith & Nelson:

      "Process groups are used in distributing signals - those process groups
       with the same group as the terminal are in the foreground and are
       allowed to read from the terminal.  Those in different group are
       considered in the background (and will be blocked if they attempt 
       to read)."

       "Closing the controlling terminal and changing the session group
       prevents the daemon process from receiving implicit (i.e. not sent
       by the user with the kill command) signals from the previous group
       leader (commonly a shell)".

       "Processes are organized within process groups and process groups
        within sessions.  With setsid() a new session (and thus, a new
        process group) is then created with the process (daemon) as the
        new leader".

    */  

    status = setsid();
    if (-1 == status)
    {
        perror("setsid()");
        exit(1);
    }


    /* And now what? Once we created a new process group, it re-acquires
       a controling terminal (!). To solve this, we fork() again.  The
       parent dies (as usual), the child, becomes a child of init,  then
       it is no more a process leader and doesn't get a controling terminal.
    */ 

    status = fork();
    switch (status)
    {
    case -1:
        perror("fork()");   /* Error in fork() */
        exit(1);

    case 0:                 /* (second) child process */
        break;

    default:                /* parent process */
      sdtdpid = fopen(SDTDPID,"w");
      if (sdtdpid != NULL) {
	fprintf(sdtdpid,"%d\n",status) ;
	fclose(sdtdpid);
      }
      exit(0);
    }

    /*
     * now we are in a new session and process group than the process that
     * started the daemon.  We also have no controlling terminal.
     */

    /* As "SDTD never dies" it's a good programming practice to 
       change to "/", in that way, if it is necessary to umount 
       some filesystem, it's possible
       */

    chdir("/");

    /* Umask is inherited from parents.  It's a good practice to set to 0 */
    umask(0);


    /* Some library routines assume that STDIN, STDOUT and STDERR are
       open.  We fool them, just sending everything to /dev/null
       */

    fileDesc = open("/dev/null", O_RDWR); /* stdin  */
    (void) dup(fileDesc);                 /* stdout */
    (void) dup(fileDesc);                 /* stderr */


/*********** END OF DAEMON DEFFINITION ************************************/


    /*
     * now starts the actual receiving process
     */


    /* catch slay, kill */
    signal( SIGTERM, ExitOnDemand )   ;	    

    /* register behind syslog() */
    openlog("sdt", LOG_NDELAY, LOG_OTHER)     ;
    log_msg(LOG_TCPIP,"start receiving",ALERT)  ;


    /* open the file to write the data */
    open_file( );

/************ START THE SOCKET COMMUNICATION *************************

  The following procedure (some comments also) was taken from:

      "Linux Network Programming, Part 1: BSD Sockets
       Ivan Grifith and John Nelson
       Linux Journal, 46, February 1998
       */


  /* Get a Socket for communication
     Socket will be of type AF_INET (Internet Domain Adress Format)
     Stream, SOCK_STREAM
     TCP/IP protocol, IPPROTO_TCP 

  */
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)    ;
    if (-1 == serverSocket)
      {
       char msg[80] ;
       sprintf( msg , "error on socket() = %d",errno) ;
       log_msg( LOG_TCPIP, msg, ALERT );
       log_msg( LOG_TCPIP, "calling process was killed", EMERGENCY) ;
       exit(1)             ;
    }

    /*

       Some setting needed.

       Turn off bind address checking, and allow port numbers
       to be reused - otherwise the TIME_WAIT phenomenon will
       prevent binding to these address.port combinations for
       (2 * MSL) seconds. This time, in Linux should MSL = 60 s

       SOL_SOCKET is the Socket Level (standard)
       SO_REUSEADDR, is the option name (defined in sockets.h)
       on = 1 is clear. 

     */
    on = 1;
    status = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR,
        (const char *) &on, sizeof(on));
    if (-1 == status)
      log_msg ( LOG_TCPIP, "setsockopt(...,SO_REUSEADDR,...)", ALERT);


    /*
       One more setting. 

       Now, the option is SO_LINGER.

       when connection is closed, there is a need to linger to ensure
       all data is transmitted, so turn this on also

     */
    {
        struct linger linger = { 0 };

        linger.l_onoff = 1;          /* Flag ON */
        linger.l_linger = 30;        /* (s)     */
        status = setsockopt(serverSocket, SOL_SOCKET, SO_LINGER,
            (const char *) &linger, sizeof(linger));
        if (-1 == status)
	  log_msg( LOG_TCPIP ,"setsockopt(...,SO_LINGER,...)", ALERT); 

    }

    /* 

       Now, get the Server IP number.  The name (TCP_SERVER) is defined
       in data_transfer.h in sst/include

       We should change this, writing directly the IP number in hostPtr.
       But I didn't succeed to do that. gethostbyname() does for me.

       */
 
    hostPtr = gethostbyname(hostname);
    if (NULL == hostPtr)
    {
       char msg[80] ;
       sprintf( msg , "error on gethostbyname() = %d",errno) ;
       log_msg( LOG_TCPIP, msg, ALERT);
       log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY) ;
       exit(1);
    }
    (void) memset(&serverName, 0, sizeof(serverName));
    (void) memcpy(&serverName.sin_addr, hostPtr->h_addr,hostPtr->h_length);

    /* Fill in all the information, in the serverName structure, which
       wil be used to open the connection */
    serverName.sin_family = AF_INET;
    serverName.sin_port = htons(port); /* network-order */


    /* 

       Now, 'bind' the socket to an address.  An Internet Address is
       IP:port, for instance: 123.234.12.23:12345

       */
    status = bind(serverSocket, (struct sockaddr *) &serverName,
        sizeof(serverName));
    if (-1 == status)
    {
       char msg[80] ;
       sprintf(msg , "error on bind() = %s",strerror(errno));
       log_msg( LOG_TCPIP , msg , ALERT);
       log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY) ;
       exit(1);
    }

    /* And start to 'listen' 
       BACK_LOG specifies the maximum size of the listen queue for
       pending connections.  If a connection arrives when the listen
       queue is full, it will fail with a connection refused error.
       This forms the basis for one type of denial of service attack.

       In principle SDTD will handle just ONE connection, but, I believe
       is a good programming practice (isn't it?)
       */
    status = listen(serverSocket, BACK_LOG);
    if (-1 == status)
    {
       char msg[80] ;
       sprintf( msg , "error on listen() = %d",errno);
       log_msg( LOG_TCPIP , msg , ALERT);
       log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY) ;
       exit(1);
    }

    for (;;) /* SDTD never dies... If the connection is broken from 
                the client side, SDTD will continue accepting connections
                till the end of the World */
    {
        int get_peer_error;
        struct in_addr inp = { 0 };
        struct sockaddr_in clientName = { 0 };
        int slaveSocket, clientLength = sizeof(clientName);

        /* I don't know while Griffin & Nelson do this, but they fill
           the clientName structure with 0. 
	   */
        (void) memset(&clientName, 0, sizeof(clientName));

    /* Now, we have received some call, accept() the connection */
        slaveSocket = accept(serverSocket,
		      (struct sockaddr *) &clientName, &clientLength);
        if (-1 == slaveSocket)
        {
	  char msg[80] ;
	  sprintf( msg ,"error on accept() = %d",errno);
          log_msg( LOG_TCPIP , msg, ALERT );
	  log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY) ;
          exit(1);
        }

	/* Want to know we are talking to */
        if ( -1 == (get_peer_error = getpeername(slaveSocket,
		  (struct sockaddr *) &clientName, &clientLength)))
	     {
	      char msg[80] ;
	      sprintf( msg , "error on getpeername() = %d",errno);
              log_msg( LOG_TCPIP , msg, ALERT );
	      log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY) ;
	      exit(1);
	     }

	/* Convert the IP of CLIENT_ALLOWED to Network language */
         status = inet_aton(CLIENT_ALLOWED, &inp);

	 /* Does the name of the calling host match the CLIENT_ALLOWED?

            If not, refuse the connection.  

            This is a little bit paranoic.  But I don't want suprises 
	    */
	 if (inp.s_addr ^ clientName.sin_addr.s_addr)
	 {
	   char msg[80] = "Connection refused to ";
	   strcat( msg , inet_ntoa(clientName.sin_addr));
	   log_msg( LOG_TCPIP , msg, ALERT );
		
	 } else { 

	   /* Now, simply, read.  If the transmited number of bytes,
	      don't match the lenght of the data structure, alert the
	      operator and write the log book
	      */
	   while (0 < ( status = read(slaveSocket,buffer,dm_lngth))){
	     if (status != dm_lngth){
	       char msg[80] ;
	       sprintf( msg , 
			"Transfer Failed. Total Bytes received = %d out of %d",
			status, dm_lngth);
	       log_msg( LOG_TCPIP , msg, ALERT );
	     } else save_data ( ) ;
	   }

	 }

    }

}


/* ExitOnDemand

   Catch SIGTERM
   
*/

void ExitOnDemand ( int sig_no ) {
  log_msg( LOG_TCPIP ,"Finish receiving", NOTICE)  ;
  close(serverSocket)   ;
  close(fp)             ;
  closelog ( )          ;
  _exit(0)              ;
}

/* open_file 

   open the file tu save the monitoring data

   We use a low level I/O operation.  I defined the permissions to be
   644: user rw, group and others r.

   The open options are:

       O_RDWR  (read/write)
       O_CREAT (create the file if it doesn't exist)
       O_SYNC  ("The file is opened for synchronous I/O. Any  writes
                 on  the  resulting  file  descriptor will block the
                 calling process until the data has been  physically
                 written  to  the  underlying  hardware".

                 "O_SYNC is not currently implemented (as of Linux 0.99pl7).

                 "There  are  many  infelicities  in the protocol underlying
                  NFS,  affecting  amongst  others  O_SYNC,  O_NDELAY,   and
                  O_APPEND." (from the manual pages of Linux, 
		  using man -S 2 open. I'm writing this program in Linux 
		  2.0.X, but the manuel page was written for 
		  Linux 0.99.pl7, 21 July 1993 )
       O_TRUNC  (If the file already exists it will be truncated.)

   */
void open_file (){
  int FPERM = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) ;
  fp = open(fname, O_RDWR | O_CREAT | O_SYNC | O_TRUNC , FPERM ) ;
  if (fp == -1){
    char msg[160] ;
    sprintf( msg ,"open monitoring file failed. Error = %s", strerror(errno)); 
    log_msg( LOG_FILES , msg , ALERT);
    log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY)  ;
    exit(1);
  }
}


/* close_file

   close the monitoring file

   */

void close_file ( void ) {
  close(fp);
}

/* save_data

   Save data in binary format

   */

void save_data ( void ) {
  int status, buflen = sizeof(recv_data1);
  /*  char * buf ;
  buf = (char * ) &recv_data1 ; */

  status = write(fp, buffer, buflen);
  if (status != buflen){
    char msg[80] ;
    sprintf( msg , "writing monitoring file failed.  Errort = %d", errno );
    log_msg( LOG_FILES , msg , ALERT);
    log_msg( LOG_TCPIP , "calling process was killed", EMERGENCY)  ;
    close_file ();
    exit(1);
  } 
}


