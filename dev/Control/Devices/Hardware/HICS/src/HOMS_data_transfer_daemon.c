/*
 * 
 * 
 * HOMS Data Transfer Daemon
 *      Transfer values from HICS ring buffer and copy in a local
 *      shared memory and in a binary file.
 *
 *
 * Guigue - 2022-02-25 - New semaphore to synchronize readout with python on-line display
 *          2021-11-23 - Added temperature channels
 *                       long options for the command line
 *                       save files every 1000 records 
 *          2021-10-25 - Corrected the raw data ADC channels
 *                       Prompt Calibration needs more coding.
 *          2021-09-11 -20 years after 9/11 OMG! -  Sao Paulo
 *          Based on sdtd.c developed for the SST
 *               May 1998 - Bern
 *               Jun 1998 - Campinas
 */


#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <getopt.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "data_transfer.h"

/* function prototypes */
static void ExitOnDemand ( int, siginfo_t *, void * ) ; 
void log_msg (unsigned short int, char *, unsigned short int) ;
void save_data( int )                       ;
void close_file ( void )                    ;
void open_file ( struct tm * )              ;
void close_shmem(void)                      ;

#ifdef PROMPT_CALIBRATION
void prompt_calibration(void)               ;
#endif

/* Socket and Communications Parameter definitions */
const int BACK_LOG = 5                      ;
const char ip_allow[] =  CLIENT_ALLOWED     ;
int serverSocket = 0                        ,
  on = 0                                    ,
  port = PORT                               ,
  status                                    ,
  nrec_read, counter_nrec_read=0            ,
  data_fd, shmem_fd, shmem_info_fd          ;

/* Data management */
unsigned char buffer[ACQ_SHMEM_SIZE]        ;
int dm_lngth =  sizeof(buffer)              ;
char hats_file_name[100]                    ;

/* shared memory */
unsigned char * shmem_base_ptr, * shmem_ptr ;
ring_buffer_shmem_info  * shmem_info_ptr    ;

/* Semaphore */
sem_t * sem_ptr, * onl_sem_ptr                            ;
static int foreground=0, prompt_calibration = 0, help=0 ; 

int main(int argc, char ** argv)
{
  struct sigaction catchSignal              ; 
  struct rlimit resourceLimit   = { 0 }     ;
  struct sockaddr_in serverSocket_in        ;
  
  FILE * sdtdpid                            ;
  int nbytes_read=-1, id_rsc, fileDesc      ;
  long unsigned int shmem_counter=0         ;
  char msg[100]                             ;
  struct stat pid_file_stat                 ;
  int option, option_index,  sval, onlsval  ;  
  static struct option long_options[] =
        {
	 {"foreground",  no_argument, &foreground, 1},
	 {"help", no_argument, &help, 1},	 
	 {"precal",  no_argument, &prompt_calibration, 1},
	 {0, 0, 0, 0}
        };  
  
  if (0 == stat(SDTDPID, &pid_file_stat)) {
    printf("\n\n There is a session of %s running. Exiting... \n\n",argv[0]);
    exit(1);
  }

  while (1)
    {
      option = getopt_long (argc, argv, "", long_options, &option_index);

      /* Detect the end of the options. */
      if (option == -1)
	break;
      
      switch (option)
	{
	case 0:
	  /* If this option set a flag, do nothing else now. */
	  if (long_options[option_index].flag != 0)
	    break;
	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;
	  
        case '?':
          /* getopt_long already printed an error message. */
          break;
	  
        default:
          abort ();
	}
    }

  if (help)
    {
      printf("\n\n Usage: %s [--foreground] [--precal] \n    (precal not yet implemented) \n\n",argv[0]);
      exit(0);
    }
  
  printf("\n\n Foreground = %d    Prompt Calibration = %d\n\n", foreground, prompt_calibration);
  
  if (! foreground)
    {

  /*
   *
   *   DAEMON DEFINITION 
   *   Start the operations to convert the program to a Unix Daemon.
   *
   *   The following procedure was taken from the article:
   *       "Linux Network Programming, Part 2: Creating Daemon Processes"
   *	    Ivan Grifith and John Nelson
   *	    Linux Journal, 47, March 1998
   *
   *   More comments on sst /monitoring/server/sdtd.c 
   *
   */
       
      status = fork();
      switch (status)
	{
	case -1:
	  perror("fork()");  // Cann't fork, exit
	  exit(1);

	case 0:                // child process, lives 
	  break;
	  
	default:               // parent process, dies
	  exit(0);
	}

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
      for (id_rsc = 0; id_rsc < resourceLimit.rlim_max; id_rsc++)
	(void) close(id_rsc);

      status = setsid();
      if (-1 == status)
	{
	  perror("setsid()");
	  exit(1);
	}

      status = fork();
      switch (status)
	{
	case -1:
	  perror("fork()");   // Error in fork()
	  exit(1);

	case 0:                 // (second) child process
	  break;

	default:                // parent process
	  sdtdpid = fopen(SDTDPID,"a");
	  if (sdtdpid != NULL) {
	    fprintf(sdtdpid,"%d\n",status) ;
	    fclose(sdtdpid);
	  }
	  exit(0);
	}
      
      chdir("/");

      // Umask is inherited from parents.  It's a good practice to set to 0
      umask(0);

      fileDesc = open("/dev/null", O_RDWR); // stdin
      (void) dup(fileDesc);                 // stdout
      (void) dup(fileDesc);                 // stderr

    /*
     *  END OF DAEMON DEFFINITION 
     */
    
    }

    /* 
     *  catch SIGINT, SIGTERM, SIGKILL
     */
  memset(&catchSignal, '\0', sizeof(catchSignal));
  catchSignal.sa_sigaction = &ExitOnDemand       ;
  catchSignal.sa_flags = SA_SIGINFO              ;
  sigaction(SIGTERM, &catchSignal, NULL)         ;
  sigaction(SIGINT, &catchSignal, NULL)          ;
    
  /* 
   * register before syslog()
   */
  openlog("data_transfer", LOG_NDELAY, LOG_ACQ)  ;
  log_msg(LOG_ACQ, "Open Communication with HICS", ALERT)     ;
  
  /* 
   *  Create the shared memories
   */
  
  if ( 0> (shmem_fd = shm_open(ACQ_SHMEM_BCKFILE,    // name from smem.h 
			       O_RDWR | O_CREAT ,    // read/write, create if needed 
			       ACQ_SHMEM_PERM )))    // access permissions (0664)
    {
      log_msg(LOG_ACQ,"Can't open shared memory segment",EMERGENCY);
      exit(1)             ;
    }

  ftruncate(shmem_fd,ACQ_SHMEM_SIZE ); 
    
  if ( (void *) -1  == (shmem_base_ptr = mmap(NULL, ACQ_SHMEM_SIZE , PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd,0)))
    {
      log_msg(LOG_ACQ,"Can't get segment...",EMERGENCY);
      exit(1);
    } else {
    shmem_ptr = shmem_base_ptr;
    sprintf(msg, "shared memory base address= %p , size=%dB",shmem_base_ptr, ACQ_SHMEM_SIZE);
    log_msg(LOG_ACQ,msg , NOTICE);
    sprintf(msg,"backing file: /dev/shm/%s", ACQ_SHMEM_BCKFILE);
    log_msg(LOG_ACQ, msg, NOTICE );
  }
    
  if ( 0> (shmem_info_fd = shm_open(ACQ_SHMEM_INFO_BCKFILE,    // name from smem.h 
				    O_RDWR | O_CREAT ,    // read/write, create if needed 
				    ACQ_SHMEM_PERM )))    // access permissions (0664)
    {
      log_msg(LOG_ACQ,"Can't open shared memory info segment",EMERGENCY);
      exit(1)             ;
    }

  ftruncate(shmem_info_fd,sizeof(ring_buffer_shmem_info) );    /* get the bytes */
  
  if ( (void *) -1  == (shmem_info_ptr = mmap(NULL, sizeof(ring_buffer_shmem_info) ,
					      PROT_READ | PROT_WRITE, MAP_SHARED, shmem_fd,0)))
    {
      log_msg(LOG_ACQ,"Can't get segment for shared memory info",EMERGENCY);
      exit(1);
    } else {
    sprintf(msg, "shared memory info base address= %p , size=%ld ",shmem_info_ptr, sizeof(ring_buffer_shmem_info));
    log_msg(LOG_ACQ,msg , NOTICE);
    sprintf(msg,"backing file: /dev/shm/%s", ACQ_SHMEM_INFO_BCKFILE);
    log_msg(LOG_ACQ, msg, NOTICE );
  }

  shmem_info_ptr->nrec = 0     ;
  shmem_info_ptr->shmem_ptr = shmem_base_ptr; 
  
  /* 
   * semaphore code to lock the shared mem
   */
  if ( (void *) -1 == ( sem_ptr = sem_open(ACQ_SEM_NAME, O_CREAT | O_RDWR , ACQ_SHMEM_PERM,0)))
    {
      log_msg(LOG_ACQ,"Can't open semaphore",EMERGENCY);
      exit(1);
    }

  /* 
   * semaphore code to lock the shared mem for on-line display
   */
  if ( (void *) -1 == ( onl_sem_ptr = sem_open(ONL_SEM_NAME, O_CREAT | O_RDWR , ACQ_SHMEM_PERM,0)))
    {
      log_msg(LOG_ACQ,"Can't open semaphore",EMERGENCY);
      exit(1);
    }
  
  if (foreground)
    {
      sem_getvalue(sem_ptr,&sval) ;
      printf("Semaphore Shmem = %d\n",sval);
    }
  
  if (prompt_calibration & foreground)
    //prompt_calibration();
    printf("Prompt Calibration habilitated\n");
      
  /*
   * now starts the actual receiving process
   The following procedure (some comments also) was taken from:

   "Linux Network Programming, Part 1: BSD Sockets
   Ivan Grifith and John Nelson
   Linux Journal, 46, February 1998

   More comments on sst monitoring/server/sdtd.c
   
  */

  serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)    ;
  if (-1 == serverSocket)
    {
      char msg[80] ;
      sprintf( msg , "error on socket() = %d",errno) ;
      log_msg( LOG_ACQ, msg, ALERT );
      log_msg( LOG_ACQ, "calling process was killed", EMERGENCY) ;
      exit(1)             ;
    }

  on = 1;
  if (-1 == (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *) &on, sizeof(on)))) 
    log_msg ( LOG_ACQ, "setsockopt(...,SO_REUSEADDR,...)", ALERT);
  {
    struct linger linger = { 0 };
    
    linger.l_onoff = 1;          /* Flag ON */
    linger.l_linger = 30;        /* (s)     */
    if (-1 == (setsockopt(serverSocket, SOL_SOCKET, SO_LINGER,
			  (const char *) &linger, sizeof(linger))))
      log_msg( LOG_ACQ ,"setsockopt(...,SO_LINGER,...)", ALERT); 
  }

  bzero(&serverSocket_in,sizeof(serverSocket_in)) ;
  serverSocket_in.sin_family      = AF_INET       ;
  serverSocket_in.sin_addr.s_addr = INADDR_ANY    ;
  serverSocket_in.sin_port        = htons(PORT)   ;    
  
  if (-1 == (bind(serverSocket,(struct sockaddr *) &serverSocket_in, sizeof(serverSocket_in))))
    {
      char msg[80] ;
      sprintf(msg , "error on bind() = %s",strerror(errno));
      log_msg( LOG_ACQ , msg , ALERT);
      log_msg( LOG_ACQ , "calling process was killed", EMERGENCY) ;
      exit(1);
    }

  if (-1 == (listen(serverSocket, BACK_LOG))) 
    {
      char msg[80] ;
      sprintf( msg , "error on listen() = %d",errno);
      log_msg( LOG_ACQ , msg , ALERT);
      log_msg( LOG_ACQ , "calling process was killed", EMERGENCY) ;
      exit(1);
    }

  sem_post(sem_ptr);
  
  for (;;) 
    {
      int get_peer_error;
      struct in_addr inp = { 0 };
      struct sockaddr_in clientName = { 0 };
      int slaveSocket;
      socklen_t clientLength = sizeof(clientName);

      (void) memset(&clientName, 0, sizeof(clientName));
      
      slaveSocket = accept(serverSocket,
			   (struct sockaddr *) &clientName, &clientLength);
      if (-1 == slaveSocket)
        {
	  char msg[80] ;
	  sprintf( msg ,"error on accept() = %d",errno);
          log_msg( LOG_ACQ , msg, ALERT );
	  log_msg( LOG_ACQ , "calling process was killed", EMERGENCY) ;
          exit(1);
        }

      /* Want to know who we are talking to */
      if ( -1 == (get_peer_error = getpeername(slaveSocket,
					       (struct sockaddr *) &clientName, &clientLength)))
	{
	  char msg[80] ;
	  sprintf( msg , "error on getpeername() = %d",errno);
	  log_msg( LOG_ACQ , msg, ALERT );
	  log_msg( LOG_ACQ , "calling process was killed", EMERGENCY) ;
	  exit(1);
	}
      
      status = inet_aton(CLIENT_ALLOWED, &inp);
      if (inp.s_addr ^ clientName.sin_addr.s_addr)
	{
	  char msg[80] = "Connection refused to ";
	  strcat( msg , inet_ntoa(clientName.sin_addr));
	  log_msg(LOG_ACQ , msg, ALERT );
		
	} else {

	while (0 < ( nbytes_read = read(slaveSocket,buffer,dm_lngth))){
	  
	  sem_getvalue(onl_sem_ptr,&onlsval);
	  /*	  if (foreground)
	    {
	      printf("Semaphore ONL = %d \n",onlsval);
	    }
	  */
	  if (onlsval > 0) sem_wait(onl_sem_ptr);
	  
	  if ( (nbytes_read % DATA_QUARK_SIZE) != 0){
	    char msg[80] ;
	    sprintf( msg , "%d bytes transferred, not an integer number of records", nbytes_read);
	    log_msg(LOG_ACQ , msg, ALERT );
	  }
	  
	  memcpy(shmem_ptr, buffer, nbytes_read);
	  nrec_read = nbytes_read / DATA_QUARK_SIZE;
	  counter_nrec_read += nrec_read ;	      
	  shmem_ptr += nbytes_read;
	  shmem_counter += nbytes_read ;
	  
	  if (shmem_counter >= MAX_BYTES_TO_BLOCK)
	    {
	      if (foreground)
		{
		  sem_getvalue(sem_ptr,&sval) ;
		  printf("Semaphore Shmem = %d Records = %d\n",sval,counter_nrec_read);
		}
		 
	      save_data(shmem_counter);
	      sem_post(sem_ptr);

	      shmem_info_ptr->nrec = counter_nrec_read ;
	      sem_post(onl_sem_ptr); 
	      
	      shmem_ptr = shmem_base_ptr;
	      shmem_counter = 0 ;
	      counter_nrec_read = 0;
	      //log_msg(LOG_ACQ,"Ring Buffer full", NOTICE);
	    }
	}
      }
    }
}


/* ExitOnDemand

   Catch SIGTERM
   
*/

static void ExitOnDemand ( int sigcode, siginfo_t * siginfo, void *context  ) {
  log_msg(LOG_ACQ ,"HOMS closed communication with HICS", NOTICE)  ;    
  close_shmem()         ;
  close(serverSocket)   ;
  close_file()          ;
  log_msg(LOG_ACQ ,"HOMS data transfer finished", NOTICE)  ;        
  closelog ( )          ;
  remove(SDTDPID)       ;
  _exit(0)              ;
}


void open_file ( struct tm * time_now_cal ){
  char msg[120], hats_full_file_name[120];
  int FPERM = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
  int OPENS ;
  struct stat filestat;
  
  sprintf(hats_file_name, "hats-%.4d-%.2d-%.2dT%.2d00.rbd",
	  time_now_cal->tm_year+1900,
	  time_now_cal->tm_mon+1,
	  time_now_cal->tm_mday,
	  time_now_cal->tm_hour);
  sprintf(hats_full_file_name,"%s/%s",DATA_DIR,hats_file_name);
  
  if (-1 == stat(hats_full_file_name, &filestat))
    {
      OPENS = O_RDWR | O_CREAT | O_SYNC | O_TRUNC ;
    } else {
      OPENS = O_APPEND | O_RDWR | O_SYNC ;
    }
  
  data_fd = open(hats_full_file_name, OPENS , FPERM ) ;  
  if (data_fd == -1){
    char msg[160] ;
    sprintf( msg ,"open RBD file %s failed. Error = %s", hats_full_file_name,strerror(errno)); 
    log_msg(LOG_FILES , msg , ALERT);
    log_msg(LOG_ACQ , "calling process was killed", EMERGENCY)  ;
    exit(1);
  } else {
    sprintf( msg ,"opened RBD file  %s", hats_file_name); 
    log_msg(LOG_FILES , msg , NOTICE);
  }
}

/* 
   save_data

   Save data in binary format
*/

void save_data ( int nbytes_read) {
  int writ,sval;
  static unsigned char first=1;
  static int old_mon=0, old_mday=0, old_hour=0;
  time_t time_now;
  struct tm * time_now_cal;

  time_now = time(NULL);
  time_now_cal = gmtime(&time_now);
  
  if (first==1) {
    open_file(time_now_cal);
    first = 0;
    old_mon   = time_now_cal->tm_mon       ;
    old_mday  = time_now_cal->tm_mday      ;
    old_hour  = time_now_cal->tm_hour      ;
  } else {
    if ( (old_mon  != time_now_cal->tm_mon   ) ||
	 (old_mday != time_now_cal->tm_mday  ) ||
	 (old_hour != time_now_cal->tm_hour  ) )
      { 
	close_file();                    // close open file
	open_file(time_now_cal);
        old_mon   = time_now_cal->tm_mon       ;
        old_mday  = time_now_cal->tm_mday      ;
        old_hour  = time_now_cal->tm_hour      ;
      }
  }
  

  if ( ! sem_wait(sem_ptr) )
    {
      if (foreground)
	{
	  sem_getvalue(sem_ptr,&sval) ;
	  printf("Semaphore Shmem = %d\n",sval);
	}
      writ = write(data_fd, shmem_base_ptr, nbytes_read);
      if (writ != nbytes_read){
	char msg[80] ;
	sprintf( msg , "writing RBD file failed.  Errort = %d", errno );
	log_msg( LOG_FILES , msg , ALERT);
	log_msg( LOG_ACQ , "calling process was killed", EMERGENCY)  ;
	close_file ();
	exit(1);
      }
    }
  
}
  

/* 
   close_file

   close the monitoring file

*/

void close_file (void ) {
  char msg[120];
  close(data_fd);
  sprintf( msg, "closed RBD file %s", hats_file_name) ;  
  log_msg( LOG_FILES, msg, ALERT);  
}

/*
  close_shmem
  Close the shared memory and the semaphore. Unlink the backing file.
*/

void close_shmem(void){
  munmap(shmem_base_ptr, ACQ_SHMEM_SIZE);
  close(shmem_fd);
  close(shmem_info_fd);
  log_msg(LOG_ACQ,"shared memories closed",NOTICE);  
  sem_close(sem_ptr);
  sem_close(onl_sem_ptr);
  log_msg(LOG_ACQ,"semaphore memories closed",NOTICE);  
  shm_unlink(ACQ_SHMEM_BCKFILE);
  shm_unlink(ACQ_SHMEM_INFO_BCKFILE);
  sem_unlink(ACQ_SEM_NAME);
  sem_unlink(ONL_SEM_NAME);  
  log_msg(LOG_ACQ,"shared memories cleaned up",NOTICE);
}
  

#ifdef PROMPT_CALIBRATION
void prompt_calibration(void)
{
#include <complex.h>
#include "windowed_dft.h"

  /*----------------------------------------------------------------------------------------------------------------------------
    Example processing real world data from HATS and outputting to a file ONLY a single frequency amplitude using goertzel algorithm
    or the FFTW whole spectrum approach.

    Description: 
    This algorithm outputs the amplitude of a single frequency and how it changes over time. 
    
    Programmed by Guillermo Giménez de Castro and Manuel Giménez de Castro in 2021.
    --------------------------------------------------------------------------------------------------------------------------*/

#define target_frequency 20  /*frequency that we are interested*/
#define window_size 256      /*number of sample point that compose each DFT analysis*/
#define steps 128            /*number of "walked" sample points for each window*/

  int status, nrec=0  ;
  
  status = fork();
  switch (status)
    {
    case -1:
      perror("fork()");  // Cann't fork, exit
      exit(1);

    case 0:                // child process, it lives
        break;

    default:               // parent process, returns
      return;
    }
  
  HICS_DATA_STR  * data_base_ptr = malloc(sizeof(TEN_MIN_MS)), * data_ptr ; 
  unsigned char buffer[ACQ_SHMEM_SIZE]  ;
  int adc, ps, i;
  unsigned long int N;
  unsigned husec;
  
  data_ptr = data_base_ptr;
  
  for (;;){
    
    if ( ! sem_wait(sem_ptr) )
      {
	memcpy(data_ptr,shmem_ptr,shmem_info_ptr->nrec*DATA_QUARK_SIZE);

	for (i=0;i<shmem_info_ptr->nrec;i++){
	 
	  memcpy(&N,buffer+i*DATA_QUARK_SIZE,4);
	  memcpy(&husec,buffer+10+i*DATA_QUARK_SIZE,8);
       
	  memcpy(&adc,buffer+18+i*DATA_QUARK_SIZE,4);
	  adc = adc & 0x00FFFFFF;
	  if ((adc & 0x0800000)>0) adc-=0x1000000;

	  memcpy(&ps,buffer+22+i*DATA_QUARK_SIZE,4);
	  ps = ps & 0x00FFFFFF;
	  if ((ps & 0x0800000)>0) ps-=0x1000000;

	  (data_ptr+i)->sample  = N     ;
	  (data_ptr+i)->husec   = husec ;
	  (data_ptr+i)->adcuGol = adc   ;
	  (data_ptr+i)->adcuPS  = ps    ; 
	}
       
	data_ptr+=shmem_info_ptr->nrec;
	nrec+=shmem_info_ptr->nrec;
	if (nrec >TEN_MIN_MS){
	  nrec=0;
	  data_ptr=data_base_ptr;
	}
	
      }
    
  }

  //fclose(testfile);

  return;
}
#endif
