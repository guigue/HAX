/*
	==============================================================
	Universidade Presbiteriana Mackenzie
	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
	==============================================================

	GetPositionLoop_v0.2
	---------------------------------------------------
	Versão contendo todos os parametros selecionados para serem extraidos da
	montagem Paramount. Nesta versão os dados são apenas mostrados na tela.
	Utilizadas classes:
	sky6RASCOMTele e sky6ObjectInformation
	---------------------------------------------------

	Autor: Tiago Giorgetti
	Email: tiago.giorgetti@craam.mackenzie.br

	Histórico:
	_______________________________________________________________________________
	 Versão	|  Data		|	Atualização
	-------------------------------------------------------------------------------
	  0.1	|  06-10-2019	| Primeira versão.
	-------------------------------------------------------------------------------
	  0.2   |  31-10-2019   | Inclusão de informações do objeto e itens de tela
		|               | contadores de vetores e definicao da estrutura
	________|_______________|______________________________________________________

	Implementação do kbhit() na referencia baixo:
	https://www.raspberrypi.org/forums/viewtopic.php?t=188067 - acesso em 04-10-2019.

*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <stdint.h>
#include <inttypes.h>

#include <semaphore.h>  //For Shared Memory
#include <sys/mman.h>   //For Shared Memory
#include <sys/stat.h>   //For Shared Memory

#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //open(sock) or file
#include <unistd.h>     //close(sock) or write to a file descriptor

#include <time.h> 	//usleep() to timing socket message transfer

#include "getPos.h"

        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
        *****************************************************************/

int get_data_from_mount(int, char **, int, double *);
uint64_t husec_time(void);
bool kbhit(void);
void report_and_exit(const char *);

        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

        /****
	 *   Global Variables
         ***************************************************************/


int main(int argc , char *argv[])
{

  pos_data_type * rbpos_base_ptr, * rbpos_ptr ;
  size_t ByteSize = sizeof(pos_data_type) * RINGSIZE	;	//Used for Shared Memory and to Write in file
  int fd_shmem						;	//Used for Shared Memory
  sem_t * semptr					;	//Used for Shared Memory

  int  sock							;	//Socket variable
  struct sockaddr_in server					;	//Socket variable

  double sidereal, jd, alt, az, ra, dec, RA_trackingrate, DEC_trackingrate;
    
  unsigned long long rb_ctr = 1				;	//Ring Buffer Counter

  int fd_data, fd_data_w                                ;	//Used to Open and Write a File
  char *filename = DATAFILENAME                         ;	//Used to Open/Create a File

  int command_nlines = 7;              // getter commands number of lines.
                                        // if you add a command remember to modify, add, etc a new
                                        // command_nlines
  char * get_ALT []  =
    {
      "/* Java Script */",
      "/*  Socket Start Packet */",
      "var Out;",
      "sky6RASCOMTele.GetAzAlt();",
      "var alt = sky6RASCOMTele.dAlt;",
      "Out = alt",
      "/* Socket End Packet */",
      "!"	//End Caracter
    };
  
  char * get_AZ[] = 
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
      "sky6RASCOMTele.GetAzAlt();",
      "var az = sky6RASCOMTele.dAz;",
      "Out = az",
      "/* Socket End Packet */",
      "!"     //End Caracter
    };

  char* get_RA[] =
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
      "sky6RASCOMTele.GetRaDec();",
    "var ra = sky6RASCOMTele.dRa;",
      "Out = ra",
      "/* Socket End Packet */",
      "!"	//End Caracter
    };

  char* get_DEC[] =
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
      "sky6RASCOMTele.GetRaDec();",
      "var dec = sky6RASCOMTele.dDec;",
      "Out = dec",
      "/* Socket End Packet */",
      "!"     //End Caracter
    };
  

  char* get_RA_TrackingRate[] =
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
    "sky6RASCOMTele.GetRaDec();",
      "var tra = sky6RASCOMTele.dRaTrackingRate;",
      "Out = tra",
      "/* Socket End Packet */",
      "!"	//End Caracter
    };
  
  
  char* get_DEC_TrackingRate[] =
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
      "sky6RASCOMTele.GetRaDec();",
      "var tdec = sky6RASCOMTele.dDecTrackingRate;",
      "Out = tdec",
      "/* Socket End Packet */",
      "!"     //End Caracter
    };

  char* get_object_sidereal[] =
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
      "sky6ObjectInformation.Property(173);",
      "var sidereal = sky6ObjectInformation.ObjInfoPropOut;",
      "Out = sidereal",
      "/* Socket End Packet */",
      "!"     //End Caracter
    };
  
  char* get_object_jd[] = 
    {
      "/* Java Script */",
      "/* Socket Start Packet */",
      "var Out;",
      "sky6ObjectInformation.Property(174);",
      "var jd = sky6ObjectInformation.ObjInfoPropOut;",
      "Out = jd",
      "/* Socket End Packet */",
      "!"     //End Caracter
    };
  
//----------End--Of--Javascripts--------------------------------------------

  /***** S H A R E D  M E M O R Y *****/

  fd_shmem = shm_open(BackingFile,      	/* name from smem.h */
		      O_RDWR | O_CREAT, 	/* read/write, create if needed */
		      AccessPerms);   	  	/* access permissions (0644) */
  
  if (fd_shmem < 0) report_and_exit("Can't open shared mem segment...");
  
  ftruncate(fd_shmem, ByteSize); 		/* get the bytes */
  
  rbpos_base_ptr = mmap(NULL, 			        /* let system pick where to put segment */
			ByteSize,   			/* how many bytes */
			PROT_READ | PROT_WRITE,		/* access protections */
			MAP_SHARED, 			/* mapping visible to other processes */
			fd_shmem,      			/* file descriptor */
			0);         			/* offset: start at 1st byte */
  
  if ( (void *) -1  == rbpos_base_ptr)
    report_and_exit("Can't get segment for shared memory...");
  else
    rbpos_ptr = rbpos_base_ptr;
  
  /**  Semaphore code to lock the shared mem  **/
  semptr = sem_open(SemaphoreName, 	/* name */
		    O_CREAT,       			/* create the semaphore */
		    AccessPerms,   			/* protection perms */
		    0);            			/* initial value */
  
  if (semptr == (void*) -1) report_and_exit("sem_open");
  
  /***** F I L E  O P E N  *****/

  if ( (fd_data = open(filename, O_RDWR | O_CREAT | O_APPEND, AccessPerms)) == -1)
    {
      fprintf(stderr, "Cannot open getposition data file. Try again later.\n");
      exit(1);
    }
  
  /***
   *     I N F I N I T E    L O O P
   *****************************************************************/
  
  while(!kbhit())
    {

      //FIRST	SOCKET	//Create socket 1
      sock = socket(AF_INET , SOCK_STREAM , 0);
      if (sock == -1)
	{
	  printf("Could not create socket");
	}
      //printf("Socket created\n");
      
      server.sin_addr.s_addr = inet_addr(IP_SERVER);
      server.sin_family = AF_INET;
      server.sin_port = htons(TCP_PORT);
      
      //Connect to remote server
      if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
	  printf("Connect failed. Error\n");
	  return 1;
	}
      
      //------------------------------------------------------------------
      //------------ Altitude Coordinate GET Block Code ------------------

      if ( get_data_from_mount(sock, get_ALT, command_nlines, &alt) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");

      
      //------------------------------------------------------------------
      //------------- Azimute Coordinate GET Block Code ------------------
      
      if ( get_data_from_mount(sock, get_AZ, command_nlines, &az) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      //------------------------------------------------------------------
      //----------- Right Ascension (RA) Coordinate GET Block Code -------
      
      if ( get_data_from_mount(sock, get_RA, command_nlines, &ra) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      
      //------------------------------------------------------------------
      //---------- Declination (DEC) Coordinate GET Block Code -----------
      
      if ( get_data_from_mount(sock, get_DEC, command_nlines, &dec) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      //------------------------------------------------------------------
      //--------- Tracking Rate (RA) GET Block Code ----------------------
      
      if ( get_data_from_mount(sock, get_RA_TrackingRate, command_nlines, &RA_trackingrate) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      
      //=================================================================
      close(sock);
      //=================================================================
      
      
      //SECOND SOCKET  //Create socket 2
      sock = socket(AF_INET , SOCK_STREAM , 0);
      if (sock == -1)
	{
	  printf("Could not create socket");
	}
      //printf("Socket created\n");
      
      server.sin_addr.s_addr = inet_addr(IP_SERVER);
      server.sin_family = AF_INET;
      server.sin_port = htons(TCP_PORT);
      
      //Connect to remote server
      if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
	  printf("Connect failed. Error\n");
	  return 1;
	}
      
      //------------------------------------------------------------------
      //--------- Tracking Rate (DEC) GET Block Code ---------------------
      
      if ( get_data_from_mount(sock, get_DEC_TrackingRate, command_nlines, &DEC_trackingrate) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      //------------------------------------------------------------------
      //------------ Sidereal Time Object GET Block Code -----------------

      if ( get_data_from_mount(sock, get_object_sidereal, command_nlines, &sidereal) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      //------------------------------------------------------------------
      //------------- Julian Date Object GET Block Code ------------------
      if ( get_data_from_mount(sock, get_object_jd, command_nlines, &jd) )
	report_and_exit( (const char *) "\n Problems reading data from Mount\n");
      
      //=================================================================
      close(sock);
      //=================================================================

      //------------------------------------------------------------------
      //------------- Ring Buffer Storage Block Code ---------------------
      
      rbpos_ptr->time_Husec       = husec_time()     ;
      rbpos_ptr->time_JD          = jd               ;
      rbpos_ptr->time_Sid         = sidereal         ;
      rbpos_ptr->pos_tele_alt     = alt              ;
      rbpos_ptr->pos_tele_az      = az               ;
      rbpos_ptr->pos_tele_ra      = ra               ;
      rbpos_ptr->pos_tele_dec     = dec              ;
      rbpos_ptr->rate_ObjId_ra    = RA_trackingrate  ;
      rbpos_ptr->rate_ObjId_dec   = DEC_trackingrate ;
      
      //------------------------------------------------------------------
      //------ Increment the semaphore so that memreader can read  -------
      
      if (sem_post(semptr) < 0) report_and_exit("sem_post");
      
      if ((rb_ctr%RINGSIZE) == 0)
	{
	  rbpos_ptr = rbpos_base_ptr;  
	  if ( (fd_data_w = write(fd_data , rbpos_base_ptr , ByteSize)) < ByteSize)
	    {
	      perror("Problems writing the file");
	      exit(1);
	    }
	} else {
	rbpos_ptr++;
      }
      rb_ctr++;      
      
      //------------------------------------------------------------------
      //----------- L O O P   E N D --------------------------------------


    }
  fprintf(stderr,"\n\n\n Bye \n\n");
  munmap(rbpos_base_ptr, ByteSize); /* unmap the storage */
  close(fd_shmem);
  sem_close(semptr);
  shm_unlink(BackingFile); /* unlink from the backing file */

  close(fd_data);
  printf("\n");
  return 0;
}

int get_data_from_mount(int sock, char *get_command[], int command_nlines, double * val){

  char server_reply[RCV_BUFFER_SIZE] ;
  int i;
  
  // Send get data
  for(i = 0; i<command_nlines ; i++)
    {
      if( send(sock , get_command[i] , strlen(get_command[i]) , 0) < 0)
	{
	  puts("Send failed. Error");
	  return -1;
	}
      usleep(TX_DELAY);
    }
  
  //Receive a reply from the server
  if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
    {
      printf("Recv failed. Error\n");
      return -1;
    }
  
  // Return the value in a double
  *val = atof (server_reply) ;
  return 0;

}


/*** Time Hundred of Micro Seconds (Husec) ***/

uint64_t husec_time(void)
{
	/****************************************************************************************

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

	uint64_t all;
	time_t sec;
	struct timespec spec;

	clock_gettime(CLOCK_REALTIME, &spec);
	/*  Convertion procedure  */
	sec = spec.tv_sec % ONEDAY;   	// get the number of seconds of the present day getting
               				// the remainder of the division by 86400 (total number of seconds in a day)

	all = (uint64_t) sec * SEC2HUSEC + (uint64_t) spec.tv_nsec / HUSEC2NSEC; // convert seconds to husecs
	                                                                         // convert nanoseconds of the present second to husecs
               		                                                         // and get the total

	return all;
}

/* KeyBoard Hit */

bool kbhit(void)
{

	struct termios original;
	tcgetattr(STDIN_FILENO, &original);
	struct termios term;
	memcpy(&term, &original, sizeof(term));
	term.c_lflag &= ~ICANON;
	tcsetattr(STDIN_FILENO, TCSANOW, &term);
	int characters_buffered = 0;
	ioctl(STDIN_FILENO, FIONREAD, &characters_buffered);
	tcsetattr(STDIN_FILENO, TCSANOW, &original);
	bool pressed = (characters_buffered != 0);
	fprintf(stderr,"\n\n\n kbhit\n\n");
	return pressed;
}


void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}
