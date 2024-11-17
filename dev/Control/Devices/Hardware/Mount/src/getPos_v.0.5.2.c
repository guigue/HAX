/*
	================================================================================
	                     Universidade Presbiteriana Mackenzie
	         Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
	================================================================================

	GetPositionLoop_v0.5
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
	--------------------------------------------------------------------------------
	  0.5   |               |
	--------------------------------------------------------------------------------
	  0.6	|  15-04-2021	| Adequacao para utilizaçao do codigo cfgCatcher para 
                |		| acessar a memoria compartilhada com as configuracoes.
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
#include <arpa/inet.h>  //inet_add 
#include <fcntl.h>      //open(sock) or file
#include <unistd.h>     //close(sock) or write to a file descriptor
#include <time.h> 	//usleep() to timing socket message transfer
#include <stdarg.h>     //Used to logging routine in the end of this code

#include "log.h"        //Used to logging routine 
#include "getPos.h"	//Definitions for getPos
#include "husec.h"	//Time Hundred of Micro Seconds (Husec) 

#include "cfg_buffer.h" //Atualizacao para versao 0.6 - cfgCatcher


#define AccessPermsIN 0644
//#define GetPos_AccessPerms 0644 
//#define csize 50


        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
        *****************************************************************/

bool kbhit(void);
void report_and_exit(const char *);




        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

int main(int argc , char *argv[])
{

	// DECLARACAO VARIAVEIS DA FUNCAO PRINCIPAL
	// ----------------------------------------
	
	clock_t begin,end,Dtime;
	
	// ----Config Data Shared Memory
	cfgBuffer_data * config_var				;	//Structure for config data from Shared Memory
	size_t ByteSize_cfg = sizeof(cfgBuffer_data)		;	//Used for Config data from Shared Memory
	int fd_shmem_cfg					;	//Used for Config data from Shared Memory
	sem_t * semptr_cfg					;	//Used for Config data from Shared Memory

	static char * BackingFileIN = "HAX-ConfigBuffer"	;	//Used for Config data from Shared Memory
	static char * SemaphoreNameIN = "HAX-ConfigSemaphore"	;	//Used for Config data from Shared Memory

	static char IP_SERVER[csize]				;	//From config file - General Config
	static long int TCP_PORT 				;	//From config file - General Config
	static long int RCV_BUFFER_SIZE 			;	//From config file - General Config
	static long int TX_DELAY				;	//From config file - General Config
	static char DIRECTORY_LOG[csize]			;	//From config file - General Config
	static char GetPos_BackingFile[csize] 			;	//From config file - Specific
	static char GetPos_SemaphoreName[csize]			;	//From config file - Specific
	static long int GetPos_AccessPerms 			;	//From config file - Specific
	static long int RINGSIZE				;	//From config file - Specific
	static char DATAFILENAME[csize]				;	//From config file - Specific



	// C O N F I G   D A T A   S H A R E D   M E M O R Y
	// -------------------------------------------------
	
	fd_shmem_cfg = shm_open(BackingFileIN, O_RDWR, AccessPermsIN);		//Empty to begin

	if (fd_shmem_cfg < 0)
	{
		report_and_exit("Can't get file descriptor for configuration data.");
	}

	// Get a pointer to memory
	config_var = mmap(NULL,
			ByteSize_cfg,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd_shmem_cfg,
			0);
	
	if ((void *) -1 == config_var)
	{
		report_and_exit("Can't access segment for shared memory for configuration data.");
	}

	// Create a semaphore for mutual exclusion
	semptr_cfg = sem_open(SemaphoreNameIN,
			O_CREAT,
			AccessPermsIN,
			0);
	
	if (semptr_cfg == (void*) -1)
	{
		report_and_exit("Can't open semaphore for config data");
	}


	// Use semaphore as a mutex (lock) by waiting for writer to increment it
	if (!sem_wait(semptr_cfg))	//Wait until semaphore != 0
	{
	
		strncpy(IP_SERVER,config_var->IP_SERVER,csize);
		strncpy(DIRECTORY_LOG,config_var->DIRECTORY_LOG,csize);
		strncpy(GetPos_BackingFile,config_var->GetPos_BackingFile,csize);
		strncpy(GetPos_SemaphoreName,config_var->GetPos_SemaphoreName,csize);
		strncpy(DATAFILENAME,config_var->DATAFILENAME,csize);

		TCP_PORT 		= config_var->TCP_PORT; 
		RCV_BUFFER_SIZE 	= config_var->RCV_BUFFER_SIZE;
		TX_DELAY 		= config_var->TX_DELAY;
		GetPos_AccessPerms 	= config_var->GetPos_AccessPerms;
		RINGSIZE 		= config_var->RINGSIZE;
	
		sem_post(semptr_cfg);
	}

	//Cleanup
	munmap(config_var, ByteSize_cfg);
	close(fd_shmem_cfg);
	sem_close(semptr_cfg);



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





	// - - - - - - - - - - - - - - - - - - - - - - 

	// R I N G   B U F F E R    -    R O U T I N E

	// - - - - - - - - - - - - - - - - - - - - - - 



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



	
	//   R I N G    B U F F E R    S H A R E D    M E M O R Y  
	// ------------------------------------------------------

	fd_shmem = shm_open(GetPos_BackingFile,      	// name from smem.h 
		      O_RDWR | O_CREAT, 		// read/write, create if needed 
		      GetPos_AccessPerms);   	  	// access permissions (0644) 

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
	semptr = sem_open(GetPos_SemaphoreName, 	// name 
			O_CREAT,       			// create the semaphore 
			GetPos_AccessPerms,   		// protection perms 
			0);            			// initial value 

	if (semptr == (void*) -1)
	{
		report_and_exit("sem_open");
		log_info("sem_open");
	}

	/** File Open **/
	if ( (fd_data = open(filename, O_RDWR | O_CREAT | O_APPEND, GetPos_AccessPerms)) == -1)
	{
		fprintf(stderr, "Cannot open getposition data file. Try again later.\n");
		log_error("Cannot open getposition data file. Try again later.");
		exit(1);
	}



	/***
	*     I N F I N I T E    L O O P
	*****************************************************************/

	begin = clock();
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

		// Incluir aqui a gravacao do OPMODE proveniente da shared memory no contexto do getpos


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
			end = clock();
			//Dtime = 1E6*(end-begin)/CLOCKS_PER_SEC;
			Dtime = 1E6*(end-begin)/CLOCKS_PER_SEC;

			begin = end;
			printf("TEMPO = %20.10f | CLOCKS_PER_SEC = %ld | Begin = %20.10f | End = %20.10f \n",(double)Dtime,CLOCKS_PER_SEC,(double)begin,(double)end);




		} else {
			rbpos_ptr++;
		}
		rb_ctr++;
		
		//------------------------------------------------------------------
		//----------- L O O P   E N D --------------------------------------

	//begin = clock();

	}

	close(fd_data);
	fclose(fp);
	//printf("\n");
	return 0;
}





        /***
        *     O T H E R S    F U N C T I O N S
        *****************************************************************/



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


