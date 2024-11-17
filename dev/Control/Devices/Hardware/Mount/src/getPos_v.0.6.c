/*
	================================================================================
	                     Universidade Presbiteriana Mackenzie
	         Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
	================================================================================

	GetPositionLoop versai 0.6
*/
#define VERSION "0.6"
/*
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
	  0.5   |  13-10-2021   | Inclusao de recurso de getops para opcoes basicas
	--------------------------------------------------------------------------------
	  0.6	|  13-10-2021	| Adequacao para utilizaçao do codigo cfgCatcher para 
                |		| acessar a memoria compartilhada com as configuracoes.
		|		| Suporte a coleta dos modos de operacao. Implementacao 
		|		| de recurso --verbose para impressao do status do 
		|		| telescopio na tela. Versao estavel com gravacao nao 
		|		| rotativa em arquivo binario.
	--------------------------------------------------------------------------------
	  0.7	| 
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
#include <getopt.h>

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
#include "opmode.h"	//Operation Mode definitions

#include "cfg_buffer.h" //Atualizacao para versao 0.6 - cfgCatcher


#define AccessPermsIN 0644
//#define GetPos_AccessPerms 0644 
//#define csize 50


        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
        *****************************************************************/

bool kbhit(void);
void report_and_exit(const char *);




	// Tratamento de parametros via getopt()
	// -------------------------------------

//Opt Flag Variables
static int version_flag		;
static int debug_flag		;
int verbose_flag = 0		;
int help_flag = 0		;
int clock_flag = 0		;	//Loop time measures

//Opt Arguments
char *argument = NULL		;
int optflag_ctr	= 0		;






        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

int main(int argc , char *argv[])
{
	//Tratamento de parametros via getopt()
	//-------------------------------------
	
	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			// These options set a flag
			{"version", no_argument,	&version_flag, 	1},
			{"debug",   no_argument,	&debug_flag,	1},
			// These options don't set a flag. We distinguish them by their indices
			{"verbose", no_argument,	0,		'v'},
			{"help",    no_argument,	0,		'h'},
			{"clock",   no_argument,	0,		'c'},
			{0, 0, 0, 0}
		};

                // getopt_long stores the option index here.
                int option_index = 0;

		c = getopt_long (argc, argv, "vhc", long_options, &option_index);
		// Detect the end of the options. 
		if (c == -1)
			break;
		switch (c)
		{
			case 0:
			// If this option set a flag, do nothing else now. 
			if (long_options[option_index].flag != 0)
				break;
			
			case 'v':
				verbose_flag = 1;
				optflag_ctr++;
				//printf("verbose_flag: %d\n",verbose_flag);
				break;
			case 'h':
				help_flag = 1;
				optflag_ctr++;
				//printf("help_flag: %d\n",help_flag);
				break;
			case 'c':
				clock_flag = 1;
				optflag_ctr++;
				//printf("clock_flag: %d\n",clock_flag);
				break;
			case '?':
				// getopt_long already printed an error message. 
				exit(1);
				break;
			default:
				abort();
		}
	}

        // Print any remaining command line arguments (not options). 
	if (optind < argc)
	{
		int diff = 0;
		argument = argv[optind];
		diff = argc - optind;
		if (debug_flag) printf("\n======== DEBUG BEGIN ========\n\nnon-option ARGV-elements: ");
		while (optind < argc)
		{
			if (debug_flag) printf("%s \n", argv[optind]);
			optind++;
		}
		if (diff >= 1)
		{
			printf("\n");
			printf("getPos: I don't have any valid arguments, only options. Try -h or --help.\n");
			exit(1);
		}
		if (debug_flag) printf("\rI don't care about this = %s                 \n",argument);
	}

	// Just printing some variables to debugging
	if (debug_flag)
	{
		printf("optind = %d\n",optind);
		printf("argc = %d\n",argc);
		printf("optflag_ctr = %d\n",optflag_ctr);
	}
	
	// Options Errors Verification
	if (argument && memcmp(argument,"-",1) == 0)
	{
		puts("getPos: Object Sintax Error! Try -h or --help.");
		exit(1);
	}
	
	if (help_flag == 1 && (argc > 2 || optflag_ctr > 1))
	{
		puts("getPos: If you need help, don't input more options or parameters! Try -h or --help.");
		exit(1);
	}
	
	if (version_flag == 1 && argc > 2)
	{
		puts("getPos: For version information must be used without another option! Try -h or --help.");
		exit(1);
	}
        
	// The execution starts here, all flags and options is stored
	if (debug_flag) printf("\nRun the getPos instructions!\n\n");
	
	// Print help text in the screen
	if (help_flag)  print_usage();
	

        //=========================
	// Print version code
	if (version_flag)
	{
		printf("-----------------------------------------\n");
		printf("Get Position - getPos version %s\n", VERSION);
		printf("-----------------------------------------\n");
		exit(0);
	}
	



	// DECLARACAO VARIAVEIS DA FUNCAO PRINCIPAL
	// ----------------------------------------
	
	clock_t begin,end,Dtime;

	// ----Printing information on screen
        long int j = 0                                                  ;       //For screen print
        int k = 0                                                       ;       //For screen print
        const char spin[4]={'|', '/', '-', '\\'}                        ;       //For screen print	
	

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

	static char OpMode_BackingFile[csize]                   ;       //From config file - Specific
	static char OpMode_SemaphoreName[csize]                 ;       //From config file - Specific
	static long int OpMode_AccessPerms                      ;       //From config file - Specific



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
		strncpy(OpMode_BackingFile,config_var->OpMode_BackingFile,csize);
		strncpy(OpMode_SemaphoreName,config_var->OpMode_SemaphoreName,csize);

		TCP_PORT 		= config_var->TCP_PORT; 
		RCV_BUFFER_SIZE 	= config_var->RCV_BUFFER_SIZE;
		TX_DELAY 		= config_var->TX_DELAY;
		GetPos_AccessPerms 	= config_var->GetPos_AccessPerms;
		RINGSIZE 		= config_var->RINGSIZE;
		OpMode_AccessPerms      = config_var->OpMode_AccessPerms;

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




	// DECLARACAO VARIAVEIS PARA OPERATION MODE - SHARED MEMORY
	// --------------------------------------------------------
	
	opmode_data_type * opmode_var                           ;       //Structure for Operation Mode data for Shared Memory
	size_t ByteSize_opmode = sizeof(opmode_data_type)       ;       //Used for OpMode data for Shared Memory
	int fd_shmem_opmode                                     ;       //Used for OpMode data for Shared Memory
	sem_t * semptr_opmode                                   ;       //Used for OpMode data for Shared Memory
	


	//      O P E R A T I O N   M O D E   -   S H A R E D   M E M O R Y
	// ----------------------------------------------------------------
	
	fd_shmem_opmode = shm_open(OpMode_BackingFile,          // name from smem.h
		O_RDWR | O_CREAT,                               // read/write, create if needed
		OpMode_AccessPerms);                            // access permissions (0644)
	
	if (fd_shmem_opmode < 0)
	{
		report_and_exit("Can't get file descriptor for configuration data.");
	}
	
	ftruncate(fd_shmem_opmode, ByteSize_opmode);            //get the bytes
	
	// Get a pointer to memory
	opmode_var = mmap(NULL,
		ByteSize_opmode,
		PROT_READ | PROT_WRITE,
		MAP_SHARED,
		fd_shmem_opmode,
		0);
	
	if ( (void *) -1  == opmode_var)
	{
		report_and_exit("Can't get segment for shared memory for configuration data.");
	}
	
	// Create a semaphore for mutual exclusion
	semptr_opmode = sem_open(OpMode_SemaphoreName,          // name
		O_CREAT,                                // create the semaphore
		OpMode_AccessPerms,                     // protection perms
		0);                                     // initial value
	
	if (semptr_opmode == (void*) -1)
	{
		report_and_exit("Can't open semaphore for config data");
	}
	
	if (sem_post(semptr_opmode) < 0)
	{
		report_and_exit("Can't increment semaphore to permit read.");
	}

        //------------------------------------
	// THIS IS THE OP_MODE ROUTINE - BEGIN
	
	// This must be included at the correct place - this is just an example
	
	// Use semaphore as a mutex (lock) by waiting for writer to increment it
/*      if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
	{
		strncpy(opmode_var->object,argv[1],csize);
		opmode_var->opmode = SLEW;
		sem_post(semptr_opmode);
	}
*/
	// THIS IS THE OP_MODE ROUTINE = END
	// ---------------------------------
	
	//Cleanup
/*      munmap(opmode_var, ByteSize_opmode);
	close(fd_shmem_opmode);
	sem_close(semptr_opmode);
*/
	//---------------------------------------------------------------	




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

	if (clock_flag) begin = clock();
	while(!kbhit())
	{


		//CREATING THE SOCKET
		sock = socket(AF_INET , SOCK_STREAM , 0);
		if (sock == -1)
		{
			printf("Could not create socket\n");
			log_fatal("Could not create socket TCP.");
		}
		log_info("Socket TCP created");
		if (debug_flag) printf("Socket TCP created.\n");

		server.sin_addr.s_addr = inet_addr(IP_SERVER);
		server.sin_family = AF_INET;
		server.sin_port = htons(TCP_PORT);

		//CONNECT TO REMOTE SERVER
		if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			printf("Could not connect to remote socket server (%s:%ld).\n", IP_SERVER, TCP_PORT);
			log_fatal("Could not connect to remote socket server (%s:%ld).", IP_SERVER, TCP_PORT);
			exit(1);
		}
		log_info("Connection stablished with the server (%s:%ld)", IP_SERVER, TCP_PORT);
		if (debug_flag) printf("Connection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);

		/************************************************************/

		//SENDING DATA TO SERVER
		for(i = 0; i<command_nlines ; i++)
		{
			if( send(sock , get_data[i] , strlen(get_data[i]) , 0) < 0)
			{
				printf("Send failed. Error\n");
				log_error("Could not send data to remote socket server.");
				exit(1);
			}
			usleep(TX_DELAY);
		}
		log_info("Data instructions sent to remote server.");
		if (debug_flag) printf("Data instructions sent to remote server.\n");

		//RECEIVE A REPLY FROM THE SERVER
		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
		{
			printf("Recv failed. Error\n");
			log_error("Could not receive data from socket server.");
			exit(1);
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
		
		rbpos_ptr->object = opmode_var->object;
		rbpos_ptr->opmode = opmode_var->opmode;

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
			if (clock_flag) end = clock();
			//if (clock_flag) Dtime = 1E6*(end-begin)/CLOCKS_PER_SEC;
			if (clock_flag) Dtime = 1E6*(end-begin)/CLOCKS_PER_SEC;

			if (clock_flag) begin = end;
			if (clock_flag) printf("TEMPO = %20.10f | CLOCKS_PER_SEC = %ld | Begin = %20.10f | End = %20.10f \n",(double)Dtime,CLOCKS_PER_SEC,(double)begin,(double)end);

		} else 
		{
			rbpos_ptr++;
		}
		rb_ctr++;
		
		//------------------------------------------------------------------
		//----------- L O O P   E N D --------------------------------------


		//------------------------------------------------------------------
		//----------- Screen Prints Block Code -----------------------------
		if(verbose_flag) system("clear"); // Clean Screen
		if(verbose_flag) printf("\n-----------------------------------\n");
		if(verbose_flag) printf("Telescope Information Status:\n");
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("server_reply: '%s'\n",server_reply);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("-----------------------------------\n\n");
		if(verbose_flag) printf("Husec     = %Lu\n",rbpos_ptr->time_Husec);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("Julian D. = %lf\n",rbpos_ptr->time_JD);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("Sideral T.= %lf\n",rbpos_ptr->time_Sid);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("Elevation = %lf\n",rbpos_ptr->pos_tele_alt);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("Azimute   = %lf\n",rbpos_ptr->pos_tele_az);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("RA        = %lf\n",rbpos_ptr->pos_tele_ra);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("DEC       = %lf\n",rbpos_ptr->pos_tele_dec);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("RA-rate   = %lf\n",rbpos_ptr->rate_ObjId_ra);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("DEC-rate  = %lf\n",rbpos_ptr->rate_ObjId_dec);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("Object    = %d\n",rbpos_ptr->object);
		if(verbose_flag) printf("-----------------------------------\n");
		if(verbose_flag) printf("OpMode    = %d\n",rbpos_ptr->opmode);
		if(verbose_flag) printf("-----------------------------------\n\n\n");
                if(verbose_flag) printf("\r Loading %c\n", spin[k]);
		if(verbose_flag) printf("\r Loop #%ld\n\n", j);
		if(verbose_flag) fflush(stdout);
		if(verbose_flag) j++;
		if(verbose_flag) k++;
		if(verbose_flag) {if (k == 4) { k = 0; }};
		if(verbose_flag) printf("\r New Position       = %llu\n",rb_ctr%RINGSIZE);
		//if(verbose_flag) printf("\r Address Position   = %lu\n",rbpos_ptr);
		if(verbose_flag) printf("\r RingBuffer Counter = %Lu\n\n\n", rb_ctr);
		if(verbose_flag) fprintf(stderr, "\r Shared mem address: %p [0..%lu]\n", rbpos_ptr, ByteSize - 1);
		if(verbose_flag) fprintf(stderr, "\r Backing file:       /dev/shm/%s\n", GetPos_BackingFile );
		//-------------------------------------------------------------------


	//if (clock_flag) begin = clock();

	}




	//=================================================================
	close(sock);
	//=================================================================
	log_info("Socket TCP closed.");
	if (debug_flag) printf("\nSocket TCP closed.\n");
	
	//Cleanup Op_Mode Shared Memory Stuff
	munmap(opmode_var, ByteSize_opmode);
	close(fd_shmem_opmode);
	sem_close(semptr_opmode);
	if (debug_flag) printf("Shared Memory cleanup ok.\n");
	//---------------------------------------------------------------
	
	fclose(fp);
	if (debug_flag) printf("File descriptor fp closed.\n");
	close(fd_data);
	if (debug_flag) printf("File descriptor fp_data closed.\n");

	if (debug_flag) printf("End of getPos.\n\n");
	
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

