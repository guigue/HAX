/*
	================================================================================
	                     Universidade Presbiteriana Mackenzie
	         Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
	================================================================================

	Start Observation 0.1
	--------------------------------------------------------------------------------
	Versão contendo todos os parametros selecionados para serem extraidos da
	montagem Paramount. Nesta versão os dados são apenas mostrados na tela.

	Utilizadas classes:
	sky6RASCOMTele e sky6ObjectInformation
	--------------------------------------------------------------------------------

	Autor: Tiago Giorgetti
	Email: tiago.giorgetti@craam.mackenzie.br

	--------------------------------------------------------------------------------

	Histórico:
	________________________________________________________________________________
	 Versão	|  Data		|	Atualização
	--------------------------------------------------------------------------------
	  0.1	|  06-10-2019	| Primeira versão.
	--------------------------------------------------------------------------------
	  0.2   |  31-10-2019   | Inclusão de informações do objeto e itens de tela
		|               | contadores de vetores e definicao da estrutura
	--------------------------------------------------------------------------------
	  0.3	|  11-03-2020	| Severas mudancas, incluindo arquivo de configuracao e
		|		| sistema de logging, se ajustando ao formato do pTrack.
	--------------------------------------------------------------------------------
	  0.4	|  28-01-2021	| Severas mudancas, incluindo arquivo de configuracao e
		|		| sistema de logging, se ajustando ao formato do pTrack.
	________|_______________|_______________________________________________________

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

#include "log.h"        //Used to logging routine in the end of this code
#include <stdarg.h>     //Used to logging routine in the end of this code
#include "getPos.h"	//Definitions for getPos
#include "confuse.h"    //Configuration file support (see: ../etc/HAX_Control.config)





        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
        *****************************************************************/

uint64_t husec_time(void);
bool kbhit(void);
void report_and_exit(const char *);


	// Logging functions
	// -----------------

static void lock(void);
static void unlock(void);
void log_set_udata(void *udata);
void log_set_lock(log_LockFn fn);
void log_set_fp(FILE *fp);
void log_set_level(int level);
void log_set_quiet(int enable);
void log_log(int level, const char *file, int line, const char *fmt, ...);



        // Preliminar logging definitions
        // ------------------------------

static struct
{
       	void *udata;
	log_LockFn lock;
        FILE *fp;
        int level;
        int quiet;
}L;

static const char *level_names[] =
{
	"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

#ifdef LOG_USE_COLOR
static const char *level_colors[] =
{
	"\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};
#endif




        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

int main(int argc , char *argv[])
{


        // CARREGA O ARQUIVO DE CONFIGURAÇÃO
        // ---------------------------------
        static char *IP_SERVER 		= NULL		;
        static long int TCP_PORT 	= 0		;
        static long int RCV_BUFFER_SIZE = 0		;
        static long int TX_DELAY 	= 0		;

        static char *BackingFile	= NULL		;
        static char *SemaphoreName	= NULL		;
        static long int AccessPerms	= 0		;

        static long int RINGSIZE	= 0		;
        static char *DATAFILENAME	= NULL		;

        static char *DIRECTORY_LOG 	= NULL		;

        cfg_t *cfg;
        cfg_opt_t opts[] =
        {
                CFG_SIMPLE_STR ("IP_SERVER", &IP_SERVER),
                CFG_SIMPLE_INT ("TCP_PORT", &TCP_PORT),
                CFG_SIMPLE_INT ("RCV_BUFFER_SIZE", &RCV_BUFFER_SIZE),
                CFG_SIMPLE_INT ("TX_DELAY", &TX_DELAY),

                CFG_SIMPLE_STR ("BackingFile", &BackingFile),
                CFG_SIMPLE_STR ("SemaphoreName", &SemaphoreName),
                CFG_SIMPLE_INT ("AccessPerms", &AccessPerms),

                CFG_SIMPLE_INT ("RINGSIZE", &RINGSIZE),
                CFG_SIMPLE_STR ("DATAFILENAME", &DATAFILENAME),

                CFG_SIMPLE_STR ("DIRECTORY_LOG", &DIRECTORY_LOG),
        
		CFG_END()
        };
	
        cfg = cfg_init(opts, 0);
        if( cfg_parse(cfg, "/opt/HAX/Control/Devices/Hardware/Mount/etc/HAX.config") == CFG_FILE_ERROR)
        {
                printf("\nCan´t open config file. \n\n");
                return 1;
        }
        cfg_free(cfg);
	//---------------------------------------



	// SISTEMA DE LOGGING
	// ------------------
        //  Setting the logging file name by year and concatenate with the DIRECTORY_LOG path
        //  Log file name format: HATS_Control_<YEAR>.log
        // ---------------------------------------------------------------------------------

        time_t current_time;
        struct tm *time_info;
        char year[5];
        int size;
        size=strlen(DIRECTORY_LOG)+5+17;   //Size of "HATS_Control_" + ".log" = 17 caracteres
        char Nome_Arquivo_Log[size];

        current_time = time(NULL);
        time_info = localtime(&current_time);
        strftime(year,5,"%Y",time_info);
        sprintf(Nome_Arquivo_Log,"%sHATS_Control_%s.log",DIRECTORY_LOG,year);


        FILE *fp;

        if ((fp=fopen(Nome_Arquivo_Log, "a"))==NULL)
        {
                printf("Can´t open/create the log file! Check directory permissions.\n\n");
                exit(1);
        }

        log_set_fp(fp);
        log_set_quiet(1);
        //-----------------------------------------------------------------------------------



	// DECLARACAO VARIAVEIS DA FUNCAO PRINCIPAL
	// ----------------------------------------

	pos_data_type * rbpos_base_ptr, * rbpos_ptr 		;	//Ponteiros do Ring Buffer		

	int  sock						;	//Socket variable
	struct sockaddr_in server				;	//Socket variable

	char * p						;	//Ring Buffer
	const char sep[2] = ";"					;	//Ring Buffer

	unsigned long long rb_ctr = 1				;	//Ring Buffer Counter

	size_t ByteSize = sizeof(pos_data_type) * RINGSIZE	;	//Used for Shared Memory and to Write in file
	int fd_shmem						;	//Used for Shared Memory
	sem_t * semptr						;	//Used for Shared Memory

	int fd_data, fd_data_w                                	;	//Used to Open and Write a Binary Data File
	char *filename = DATAFILENAME                         	;	//Used to Open/Create a Binary Data File
	char server_reply[RCV_BUFFER_SIZE] 			;	//Data received from server
	int i							;	//Just a regular counter

	int command_nlines = 17					;	// getter commands number of lines.
                                        				// if you add a command remember to modify, add, etc a new
									// command_nlines
	
								

	// Javascript instructions for TheSkyX server
	// ------------------------------------------
	char* get_data[] =						
	{
		"/* Java Script */",
		"/* Socket Start Packet */",
		"var Out;",
		"sky6RASCOMTele.GetAzAlt();",
		"var alt = sky6RASCOMTele.dAlt;",
		"var az = sky6RASCOMTele.dAz;",
		"sky6RASCOMTele.GetRaDec();",
		"var ra = sky6RASCOMTele.dRa;",
		"var dec = sky6RASCOMTele.dDec;",
		"var tra = sky6RASCOMTele.dRaTrackingRate;",
		"var tdec = sky6RASCOMTele.dDecTrackingRate;",
		"sky6ObjectInformation.Property(173);",
		"var sidereal = sky6ObjectInformation.ObjInfoPropOut;",
		"sky6ObjectInformation.Property(174);",
		"var jd = sky6ObjectInformation.ObjInfoPropOut;",
		"Out = jd + ';' + sidereal + ';' + alt + ';' + az + ';' + ra + ';' + dec + ';' + tra + ';' + tdec + ';'",
		"/* Socket End Packet */",
	};
	//--------------------------------------------




	//    S H A R E D     M E M O R Y 
	// -----------------------------------

	fd_shmem = shm_open(BackingFile,      		// name from smem.h 
		      O_RDWR | O_CREAT, 		// read/write, create if needed 
		      AccessPerms);   	  		// access permissions (0644) 

	if (fd_shmem < 0)
	{
		report_and_exit("Can't open shared mem segment...");
		log_error("Can't open shared mem segment...");
	}

	ftruncate(fd_shmem, ByteSize); 			// get the bytes 

	rbpos_base_ptr = mmap(NULL, 			// let system pick where to put segment 
			ByteSize,   			// how many bytes 
			PROT_READ | PROT_WRITE,		// access protections 
			MAP_SHARED, 			// mapping visible to other processes 
			fd_shmem,      			// file descriptor 
			0);         			// offset: start at 1st byte 

	if ( (void *) -1  == rbpos_base_ptr)
	{
		report_and_exit("Can't get segment for shared memory...");
		log_error("Can't get segment for shared memory...");
	} else
		rbpos_ptr = rbpos_base_ptr;

	/**  Semaphore code to lock the shared mem  **/
	semptr = sem_open(SemaphoreName, 		// name 
			O_CREAT,       			// create the semaphore 
			AccessPerms,   			// protection perms 
			0);            			// initial value 

	if (semptr == (void*) -1)
	{
		report_and_exit("sem_open");
		log_info("sem_open");
	}

	/** File Open **/
	if ( (fd_data = open(filename, O_RDWR | O_CREAT | O_APPEND, AccessPerms)) == -1)
	{
		fprintf(stderr, "Cannot open getposition data file. Try again later.\n");
		log_error("Cannot open getposition data file. Try again later.");
		exit(1);
	}



	/***
	*     I N F I N I T E    L O O P
	*****************************************************************/

	while(!kbhit())
	{

		//CREATING THE SOCKET
		sock = socket(AF_INET , SOCK_STREAM , 0);
		if (sock == -1)
		{
			//printf("Could not create socket");
			log_error("Could not create socket TCP.");
		}
		log_info("Socket TCP created");

		server.sin_addr.s_addr = inet_addr(IP_SERVER);
		server.sin_family = AF_INET;
		server.sin_port = htons(TCP_PORT);

		//CONNECT TO REMOTE SERVER
		if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			//printf("Connect failed. Error\n");
			log_error("Could not connect to remote socket server (%s:%d).", TCP_PORT, IP_SERVER);
			return 1;
		}
		log_info("Connection stablished with the server.");

		/************************************************************/

		//SENDING DATA TO SERVER
		for(i = 0; i<command_nlines ; i++)
		{
			if( send(sock , get_data[i] , strlen(get_data[i]) , 0) < 0)
			{
				//puts("Send failed. Error");
				log_error("Could not send data to remote socket server.");
				return -1;
			}
			usleep(TX_DELAY);
		}
		log_info("Data instructions sent to remote server.");

		//RECEIVE A REPLY FROM THE SERVER
		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
		{
			//printf("Recv failed. Error\n");
			log_error("Could not receive data from socket server.");
			return -1;
		}

		//=================================================================
		close(sock);
		//=================================================================


		// RING BUFFER FEEDING
		
		rbpos_ptr->time_Husec = husec_time();

		p = strtok(server_reply,sep);
		if ( p != NULL)
		{
			rbpos_ptr->time_JD = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->time_Sid = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->pos_tele_alt = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->pos_tele_az = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->pos_tele_ra = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->pos_tele_dec = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->rate_ObjId_ra = atof(p);
			p = strtok(NULL,sep);
		}

		if ( p != NULL)
		{
			rbpos_ptr->rate_ObjId_dec = atof(p);
		}

		log_info("Data Received from remote server.");

		//------------------------------------------------------------------
		//------------- Ring Buffer Storage Block Code ---------------------

		//------------------------------------------------------------------
		//------ Increment the semaphore so that memreader can read  -------

		if (sem_post(semptr) < 0)
		{
			report_and_exit("sem_post");
			log_info("sem_post");
		}

		if ((rb_ctr%RINGSIZE) == 0)
		{
			rbpos_ptr = rbpos_base_ptr;
			if ( (fd_data_w = write(fd_data , rbpos_base_ptr , ByteSize)) < ByteSize)
			{
				perror("Problems writing the file");
				log_error("Problems writing the Ring Buffer file");
				exit(1);
			}
		} else {
			rbpos_ptr++;
		}
		rb_ctr++;

		//------------------------------------------------------------------
		//----------- L O O P   E N D --------------------------------------
	}

	close(fd_data);
	fclose(fp);
	//printf("\n");
	return 0;
}





        /***
        *     O T H E R S    F U N C T I O N S
        *****************************************************************/



/*** Time Hundred of Micro Seconds (Husec) ***/

uint64_t husec_time(void)
{

        // CARREGA O ARQUIVO DE CONFIGURAÇÃO
        // ---------------------------------

        static long int SEC2HUSEC 	= 10000L	;
        static long int HUSEC2NSEC 	= 100000L	;
        static long int ONEDAY	 	= 86400L	;
       // static long int HUSECS2HRS 	= 36000000L	;
       // static long int MIN2HUSEC 	= 600000L	;
       // static long int MIN 		= 60L		;

	
	/*

	cfg_t *cfg;
        cfg_opt_t opts[] =
        {
                CFG_SIMPLE_INT ("SEC2HUSEC", &SEC2HUSEC),
                CFG_SIMPLE_INT ("HUSEC2NSEC", &HUSEC2NSEC),
                CFG_SIMPLE_INT ("ONEDAY", &ONEDAY),
                CFG_SIMPLE_INT ("HUSECS2HRS", &HUSECS2HRS),
                CFG_SIMPLE_INT ("MIN2HUSEC", &MIN2HUSEC),
                CFG_SIMPLE_INT ("MIN", &MIN),
                CFG_END()
        };

        cfg = cfg_init(opts, 0);
        if( cfg_parse(cfg, "/opt/HAX/Control/Devices/Hardware/Mount/etc/HAX_husec.config") == CFG_FILE_ERROR)
        {
                printf("\nCan´t open config file. \n\n");
                return 1;
        }
        cfg_free(cfg);

	*/

        // FIM DO CARREGAMENTO DO ARQUIVO DE CONFIGURACAO


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

	/**************************************************************************

	// Printout the results
	printf("Current time: %" PRIu64  " Hundred of Microseconds since 0:00 \n", all);  // husecs of the day
	printf("Current time (UTC)        : %d:%0.2d:%0.2d\n",
        	all / HUSECS2HRS,
		(all / MIN2HUSEC) % MIN ,
		((all % HUSECS2HRS) % MIN2HUSEC) / SEC2HUSEC);  // husecs converted back to hour, min and seconds
	//printf("Current time (BST = UTC-3): %s",ctime(&now));   // verification by using the internal time() function
                                                         	// time() is in local standard time (BST) !
	**************************************************************************/

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
	return pressed;
}

/* Report and Exit */

void report_and_exit(const char* msg) {
  perror(msg);
  exit(-1);
}





        /***
        *     L O G G I N G    F U N C T I O N S
        *****************************************************************/



static void lock(void)
{
        if (L.lock)
        {
                L.lock(L.udata, 1);
        }
}

static void unlock(void)
{
        if (L.lock)
        {
                L.lock(L.udata, 0);
        }
}

void log_set_udata(void *udata)
{
        L.udata = udata;
}

void log_set_lock(log_LockFn fn)
{
        L.lock = fn;
}

void log_set_fp(FILE *fp)
{
        L.fp = fp;
}

void log_set_level(int level)
{
        L.level = level;
}

void log_set_quiet(int enable)
{
        L.quiet = enable ? 1 : 0;
}

void log_log(int level, const char *file, int line, const char *fmt, ...)
{
        if (level < L.level)
        {
                return;
        }

        /* Acquire lock */
        lock();

        /* Get current time */
        time_t t = time(NULL);
        struct tm *lt = localtime(&t);

        /* Log to stderr */
        if (!L.quiet)
        {
                va_list args;
                char buf[16];
                buf[strftime(buf, sizeof(buf), "%H:%M:%S", lt)] = '\0';
                #ifdef LOG_USE_COLOR
                        fprintf(
                                stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
                                buf, level_colors[level], level_names[level], file, line);
                #else
                        fprintf(stderr, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
                #endif
                        va_start(args, fmt);
                        vfprintf(stderr, fmt, args);
                        va_end(args);
                        fprintf(stderr, "\n");
                        fflush(stderr);
        }

        /* Log to file */
        if (L.fp)
        {
                va_list args;
                char buf[160];
                buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", lt)] = '\0';
                fprintf(L.fp, "%s %-5s %s:%d: ", buf, level_names[level], file, line);
                va_start(args, fmt);
                vfprintf(L.fp, fmt, args);
                va_end(args);
                fprintf(L.fp, "\n");
                fflush(L.fp);
        }

        /* Release lock */
        unlock();
}

