/*
        =================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        =================================================================================

        Sky Dip versao 0.2
*/
#define VERSION "0.2"
/*
  	---------------------------------------------------------------------------------
        Este programa visa realizar a avaliacao da opacidade atmosferica, realizando um 
	escaneamento do ceu. Utilizando coordenadas altazimutais, o escaneamento e 
	realizado inicialmente coletando o Azimute em que o telescopio se encontra e 
	realizando um SLEW ate a Elevacao 10 graus com o Azimute corrente. Apos alcancar
	este alvo, inicia-se uma varredura de passo em passo, mantendo o Azimute fico e 
	variando a Elevacao. Os valores padrao, ou seja, sem a passagem de opcoes, 
	definem dois parametros: Step (Passo) e Time (Tempo), onde o passo e igual a 10 
	graus e o tempo 10 segundos. Enquanto o telescopio e posicionado, recebe uma 
	marca op_mode=SLEW. Estando na posicao, muda a marca para op_mode=SKY_DIP e 
	ficara aguardando 10 segundos para a realizacao da medida de opacidade e entao 
	continuara para a proxima elevacao considerando um passo de 10 graus, alterando 
	novamente a marca para op_mode=SLEW, sendo esse processo repetido ate que a 
	Elevacao seja 90 graus. Apos finalizar esse escaneamento, o telescopio ficara 
	parado com o tracking desativado, e com a marca op_mode=STALL. Tambem e possivel 
	modificar as coordenadas de Elevacao inicial e final atraves das opcoes --el_init
	e --el_end , embora caso nao sejam inseridas, por padrao os respectivos valores 
	serao 10 graus e 90 graus.

        TheSkyX used classes:
	---------------------------------------------------------------------------------
        sky6RASCOMTele
	---------------------------------------------------------------------------------

        Autor: Tiago Giorgetti
        Email: tiago.giorgetti@craam.mackenzie.br

	---------------------------------------------------------------------------------

        Histórico:
        _________________________________________________________________________________
         Versão |  Data         |       Atualização
        ---------------------------------------------------------------------------------
          0.1   |  06-10-2021   | Inicio da programacao com aproveitamento do codigo do
	  	|		| pTrack_v.0.6.5 e utilizando as opcoes -v, --verbose e 
		|		| --debug para a apresentacao de mensagens na tela. Nesta
		|		| versao apenas contem valores fixos para Passo e Tempo.
        ---------------------------------------------------------------------------------
	  0.2	|  06-10-2021	| Implementacao de opcoes para passo (step) -s e tempo 
	  	|		| (time) -t. Eliminacao de codigos remanescentes com 
		|		| funcoes que nao se aplicam ao skyDip. Implementacao
		|		| de opcoes para --el_init e --el_end, que tratam
		|		| respectivamente do valor customizado para a elevacao
		|		| inicial e final da execucao do scan.
	_________________________________________________________________________________

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
#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //open(sock) or file
#include <unistd.h>     //close(sock) or write to a file descriptor
#include <time.h>       //usleep() to timing socket message transfer
#include <stdarg.h>	//Used to logging routine in the end of this code

#include "log.h"	//Used to logging routine in the end of this code
#include "skyDip.h"	//Definitions for skyDip
#include "opmode.h"	//Definitions for Operation Modes of the Telescope
#include "cfg_buffer.h" //Atualizacao para versao 0.6 - cfgCatcher




#define AccessPermsIN 0644



        /***
        *     P R E L I M I N A R Y     F U N C T I O N S
        *****************************************************************/

void report_and_exit(const char *);
void copy_char ( char a[], char b[], char x);
int count_char ( char a[], char x);
int print_usage();     			// from skyDip.h


	// Tratamento de parametros via getopt()
	// -------------------------------------

//Opt Flag Variables
static int version_flag         ;
static int debug_flag           ;

int step_flag		= 0     ;
int time_flag		= 0     ;
int elinit_flag		= 0	;
int elend_flag		= 0	;

int help_flag           = 0     ;
int verbose_flag        = 0     ;

//Opt Arguments
char *step_value	= "10"	;	// Default Parameter for step = 10 degrees
char *time_value	= "10"  ;	// Default Parameter for time = 10 seconds
char *elinit_value	= "10"	;	// Default Parameter for Initial Elevation = 10 degrees
char *elend_value	= "90"	;	// Default Parameter for Final Elevation = 90 degrees
char *object		= NULL	;	// Not used for skyDip

int optflag_ctr         = 0     ;       // Options Flag counter





        /***
        *     M A I N    F U N C T I O N
        *****************************************************************/

int main(int argc , char *argv[])
{

	// Tratamento de parametros via getopt()
	// -------------------------------------

	int c;
	while (1)
	{
		static struct option long_options[] =
		{
			/* These options set a flag. */
			{"version", no_argument,        &version_flag,  1},
			{"debug",   no_argument,        &debug_flag,    1},
			/* These options dont't set a flag. We distinguish them by their indices. */
			{"el_init", required_argument,	0,	0},
			{"el_end", required_argument,	0,	0},
			{"step",  required_argument,	0,    	's'},
			{"time", required_argument,	0,    	't'},
			{"help",        no_argument,    0,      'h'},
			{"verbose",     no_argument,    0,      'v'},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "s:t:hv", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
	
		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				if (memcmp( long_options[option_index].name,"el_init",7)==0)
				{
					elinit_flag = 1;
					optflag_ctr++;
					//printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					elinit_value = optarg;
						//printf("%s\n",elinit_value); }
				}
				if (memcmp( long_options[option_index].name,"el_end",4)==0)
				{
					elend_flag = 1;
					optflag_ctr++;
					//printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					elend_value = optarg;
						//printf("%s\n",elend_value); }
				}
				break;
			case 's':
				step_flag = 1;
				optflag_ctr++;
				if (optarg) step_value = optarg;
				break;
			case 't':
	                	time_flag = 1;
				optflag_ctr++;
				if (optarg) time_value = optarg;
				break;
			case 'h':
				help_flag = 1;
				optflag_ctr++;
				//printf("help_flag: %d\n",help_flag);
				break;
			case 'v':
				verbose_flag = 1;
				optflag_ctr++;
				//printf("verbose_flag: %d\n",verbose_flag);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				exit(1);
				break;
			default:
				abort();
		}
	}

	
	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		int diff = 0;
		object = argv[optind];
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
			printf("skyDip: I don't have any valid arguments, only options. Try -h or --help.\n");
			exit(1);
		}
		if (debug_flag) printf("\rI don't care about this = %s                 \n",object);
	}
	
	// Just printing some variables to debugging
	if (debug_flag)
	{
		printf("optind = %d\n",optind);
		printf("argc = %d\n",argc);
		printf("optflag_ctr = %d\n",optflag_ctr);
	}



	//Resume of options activated
	
	if (debug_flag)
	{
		if(version_flag)	printf("version_flag = %d\n", version_flag);
		if(debug_flag)		printf("debug_flag = %d\n", debug_flag);
		if(step_flag) 		printf("step_flag = %d\n", step_flag);
		if(time_flag) 		printf("time_flag = %d\n", time_flag);
		if(elinit_flag) 	printf("elinit_flag = %d\n", elinit_flag);
		if(elend_flag) 		printf("elend_flag = %d\n", elend_flag);
		if(help_flag) 		printf("help_flag = %d\n", help_flag);
		if(verbose_flag) 	printf("verbose_flag = %d\n", verbose_flag);
	}

	
	// Options Errors Verification
	
	if (object && memcmp(object,"-",1) == 0)
	{
		puts("pTrack: Object Sintax Error! Try -h or --help.");
		exit(1);
	}

	if (help_flag == 1 && (argc > 2 || optflag_ctr > 1))
	{
		puts("pTrack: If you need help, don't input more options or parameters! Try -h or --help.");
		exit(1);
	}

	if (version_flag == 1 && argc > 2)
	{
		puts("pTrack: For version information must be used without another option! Try -h or --help.");
		exit(1);
	}
	

	// The execution starts here, all flags and options is stored
	if (debug_flag) printf("\nRun the skyDip instructions!\n\n");

	// Print help text in the screen
	if (help_flag) 	print_usage();


	// DECLARACAO VARIAVEIS PARA CONFIG DATA - SHARED MEMORY
	// -----------------------------------------------------
	
        cfgBuffer_data * config_var				;	//Structure for config data from Shared Memory
	size_t ByteSize_cfg = sizeof(cfgBuffer_data)		;	//Used for Config data from Shared Memory
	int fd_shmem_cfg					;	//Used for Config data from Shared Memory
	sem_t * semptr_cfg					;	//Used for Config data from Shared Memory

	static char * BackingFileIN = "HAX-ConfigBuffer"	;	//Used for Config data from Shared Memory
	static char * SemaphoreNameIN = "HAX-ConfigSemaphore"   ;	//Used for Config data from Shared Memory

	static char IP_SERVER[csize] 				;	//From config file - General Config
        static long int TCP_PORT 				;	//From config file - General Config
        static long int RCV_BUFFER_SIZE 			;	//From config file - General Config
        static long int TX_DELAY 				;	//From config file - General Config

        static char DIRECTORY_LOG[csize] 			;	//From config file - General Config
	
	static char OpMode_BackingFile[csize]			;	//From config file - Specific
	static char OpMode_SemaphoreName[csize]			;	//From config file - Specific
	static long int OpMode_AccessPerms			;	//From config file - Specific


	
	// C O N F I G   D A T A   -   S H A R E D   M E M O R Y
	// -----------------------------------------------------

	fd_shmem_cfg = shm_open(BackingFileIN, O_RDWR, AccessPermsIN);		// Empty to begin

  	if (fd_shmem_cfg < 0)
	{
		report_and_exit("Can't get file descriptor for configuration data.\nTry execute 'cfgStore' first.");
	}

	// Get a pointer to memory
	config_var = mmap(NULL,
			ByteSize_cfg,
			PROT_READ | PROT_WRITE,
			MAP_SHARED,
			fd_shmem_cfg,
			0);

	if ( (void *) -1  == config_var)
	{
		report_and_exit("Can't get segment for shared memory for configuration data.");
	}


  	// Create a semaphore for mutual exclusion
  	semptr_cfg = sem_open(SemaphoreNameIN,      	// name
        		O_CREAT,                        // create the semaphore
                	AccessPermsIN,                  // protection perms
                	0);                             // initial value

  	if (semptr_cfg == (void*) -1) 
	{
		report_and_exit("Can't open semaphore for config data");
	}

	// Use semaphore as a mutex (lock) by waiting for writer to increment it
	if (!sem_wait(semptr_cfg))		//Wait until semaphore != 0
	{
		strncpy(IP_SERVER,config_var->IP_SERVER,csize);
		strncpy(DIRECTORY_LOG,config_var->DIRECTORY_LOG,csize);
		strncpy(OpMode_BackingFile,config_var->OpMode_BackingFile,csize);
		strncpy(OpMode_SemaphoreName,config_var->OpMode_SemaphoreName,csize);

		TCP_PORT 		= config_var->TCP_PORT;
		RCV_BUFFER_SIZE 	= config_var->RCV_BUFFER_SIZE;
		TX_DELAY 		= config_var->TX_DELAY;
		OpMode_AccessPerms 	= config_var->OpMode_AccessPerms;
	
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

	//#define log_trace(...) log_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
	//#define log_debug(...) log_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
	//#define log_info(...)  log_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
	//#define log_warn(...)  log_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
	//#define log_error(...) log_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
	//#define log_fatal(...) log_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

        time_t current_time;
        struct tm *time_info;
        char year[5];
        int size;
        size = strlen(DIRECTORY_LOG)+5+17;   //Size of "HATS_Control_" + ".log" = 17 caracteres
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
	
        opmode_data_type * opmode_var				;	//Structure for Operation Mode data for Shared Memory
	size_t ByteSize_opmode = sizeof(opmode_data_type)	;	//Used for OpMode data for Shared Memory
	int fd_shmem_opmode					;	//Used for OpMode data for Shared Memory
	sem_t * semptr_opmode					;	//Used for OpMode data for Shared Memory




	//      O P E R A T I O N   M O D E   -   S H A R E D   M E M O R Y 
	// ----------------------------------------------------------------

	fd_shmem_opmode = shm_open(OpMode_BackingFile,		// name from smem.h
        	O_RDWR | O_CREAT,         			// read/write, create if needed
     		OpMode_AccessPerms);             		// access permissions (0644)

  	if (fd_shmem_opmode < 0)
	{
		report_and_exit("Can't get file descriptor for configuration data.");
	}

	ftruncate(fd_shmem_opmode, ByteSize_opmode);		//get the bytes

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
  	semptr_opmode = sem_open(OpMode_SemaphoreName,		// name
        		O_CREAT,				// create the semaphore
                	OpMode_AccessPerms,			// protection perms
                	0);					// initial value

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
/*	if (!sem_wait(semptr_opmode))		//Wait until semaphore != 0
	{
		strncpy(opmode_var->object,argv[1],csize);
		opmode_var->opmode = SLEW;
		sem_post(semptr_opmode);
	}
*/
	// THIS IS THE OP_MODE ROUTINE = END
	// ---------------------------------

	//Cleanup
/*	munmap(opmode_var, ByteSize_opmode);
	close(fd_shmem_opmode);
	sem_close(semptr_opmode);
*/
	//---------------------------------------------------------------





        int i					;       //Generic counter

/*	typedef struct
	{
		int targets;
		char * keyword;
	} target_map ;

	target_map keywordmap[] = {
		SKY,"sky",                      //0
		MERCURY,"mercury",              //1
		VENUS,"venus",                  //2
		EARTH,"earth",                  //3
		MARS,"mars",                    //4
		JUPITER,"jupiter",              //5
		SATURN,"saturn",                //6
		URANUS,"uranus",                //7
		NEPTUNE,"neptune",              //8
		PLUTO,"pluto",                  //9
		MOON,"moon",                    //10
		SUN,"sun",                      //11
		AR,"AR",                        //12
		MANUAL,"Manual",		//13
		PARK,"PARK",                    //20
		UNPARK,"UNPARK",                //21
		HOME,"HOME",                    //22
		CONNECT,"CONNECT",              //23
		DISCONNECT,"DISCONNECT",        //24
		EXTRA_OBJ, "extra object",      //30
	};

	typedef struct
	{
		int opmode_index;
		char * opmode_str;
	} opmode_map ;

	opmode_map keyword_opmode_map[] = {
		TRACK,"TRACK",          //0
		SKY_DIP,"SKY_DIP",      //10
		STALL,"STALL",          //50
		SLEW,"SLEW",            //100
		PARKED,"PARKED",        //150
		OPMODEEND,"OpModeEnd",  //200
	};

*/



	//=========================
	// Print version code
	if (version_flag)
	{
		printf("---------------------------------------\n");
		printf("Sky Dip - skyDip version %s\n", VERSION);
		printf("---------------------------------------\n");
		exit(0);
	}






	// DECLARACAO VARIAVEIS DA ROTINA DE COMUNICACAO COM SERVIDOR
	// ----------------------------------------------------------

	int el 					;	//Elevation Coordinate, starts with 10 degrees
        int step_dvalue				;	//Decimal value for step_value opt parameter
	int time_dvalue				;	//Decimal value for time_value opt parameter
	int elinit_dvalue			;	//Decimal value for elinit_value opt parameter
	int elend_dvalue			;	//Decimal value for elend_value opt parameter

	int sock                                ;       //Socket variable
        struct sockaddr_in server               ;       //Socket variable
        char server_reply[RCV_BUFFER_SIZE]      ;       //Socket variable

	


	// CONVERTION OF STRING TO THEIR RELATIVE DECIMALS
	// -----------------------------------------------
	step_dvalue = atoi(step_value);
	if (debug_flag) printf("step_dvalue = %d\n", step_dvalue);

	time_dvalue = atoi(time_value);
	if (debug_flag) printf("time_dvalue = %d\n", time_dvalue);

	elinit_dvalue = atoi(elinit_value);
	if (debug_flag) printf("elinit_dvalue = %d\n", elinit_dvalue);

	elend_dvalue = atoi(elend_value);
	if (debug_flag) printf("elend_dvalue = %d\n", elend_dvalue);




	// Javascript instructions for TheSkyX server (SkyDip)
	// ------------------------------------------
	char skyDip_data[21][100]           ;	// Data to send to server
	int skyDip_data_nlines = 21         ;	// Getter commands number of lines from set_pTrack_data[].
						// If you add a command remember to modify, add, etc a new
						// set_pTrack_data_nlines



					
	// BEGIN OF SKYDIP ROUTINE WITH THE SERVER
	// ---------------------------------------

	for (el = elinit_dvalue ; el < elend_dvalue+1 ; el += step_dvalue)
	{
	
		sprintf(skyDip_data[0],"/* Java Script */					");
		sprintf(skyDip_data[1],"/* Socket Start Packet */				");
		sprintf(skyDip_data[2],"var Out;						");
		sprintf(skyDip_data[3],"var SlewComplete = 0;					");
		sprintf(skyDip_data[4],"if(!sky6RASCOMTele.IsConnected){			");
		sprintf(skyDip_data[5],"	Out='NotConnected.';				");
		sprintf(skyDip_data[6],"}else if(sky6RASCOMTele.IsParked()){			");
		sprintf(skyDip_data[7],"	Out='Parked.';					");
		sprintf(skyDip_data[8],"}else{							");
		sprintf(skyDip_data[9],"	sky6RASCOMTele.Abort();				");
		sprintf(skyDip_data[10],"	sky6RASCOMTele.GetAzAlt();			");
		sprintf(skyDip_data[11],"	var az = sky6RASCOMTele.dAz;			");
		sprintf(skyDip_data[12],"	var el = %d;					",el);
		sprintf(skyDip_data[13],"	sky6RASCOMTele.SlewToAzAlt(az, el, '');		");
		sprintf(skyDip_data[14],"	while(SlewComplete != 1){			");
		sprintf(skyDip_data[15],"                SlewComplete = sky6Web.IsSlewComplete;	");
		sprintf(skyDip_data[16],"        }						");
		sprintf(skyDip_data[17],"	sky6RASCOMTele.SetTracking(0, 1, 0, 0);		");
		sprintf(skyDip_data[18],"	Out='SkyDip.';        				");
		sprintf(skyDip_data[19],"}							");
		sprintf(skyDip_data[20],"/* Socket End Packet */				");


		// TCP Communication
		// -----------------

		//CREATING THE SOCKET
      		sock = socket(AF_INET , SOCK_STREAM , 0);
      		if (sock == -1)
       		{
			log_fatal("Could not create socket TCP.");
			report_and_exit("Could not create socket TCP.");
		}
		log_info("Socket TCP created.");
		if (debug_flag) printf("Socket TCP created.\n");

      		server.sin_addr.s_addr = inet_addr(IP_SERVER);
      		server.sin_family = AF_INET;
      		server.sin_port = htons(TCP_PORT);

    	  	//CONNECT TO REMOTE SERVER
      		if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    	    	{
          		log_fatal("Could not connect to remote socket server (%s:%ld).", IP_SERVER, TCP_PORT);
        		report_and_exit("Could not connect to remote socket server.");
        	}
		log_info("Connection stablished with the server (%s:%ld)", IP_SERVER, TCP_PORT);
		if (debug_flag) printf("Connection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);



		//SENDING DATA INSTRUCTIONS TO SERVER 
		/************************************/
      		//------------------------------------
		if (el == elinit_dvalue)
		{
			if (debug_flag) printf("\nSlewing to SkyDip position...\n\n");
			if (verbose_flag) printf("\nSlewing to SkyDip position...\n");
		}else 
		{	if (debug_flag) printf("Slewing to next SkyDip position...\n\n");
			if (verbose_flag) printf("\rSlewing to next SkyDip position...\n");
		}

		for(i = 0; i<skyDip_data_nlines ; i++)
      		{
			if( send(sock , skyDip_data[i] , strlen(skyDip_data[i]) , 0) < 0)
			{
				log_fatal("Could not send data to remote socket server.");
				report_and_exit("Could not send data to remote socket server.");
			}
			if (debug_flag) printf("%s\n",skyDip_data[i]);
       	 	}
		if (debug_flag) printf("\n");

		// Routine for Op_Mode = SLEW to PARK
		// - - - - - - - - - - - - - - -
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
		{
			//opmode_var->object = SKY;
			opmode_var->opmode = SLEW;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag)
		{
			printf("----------------------\n");
			printf("Operation Mode Set:   \n");
			printf("----------------------\n");
			//printf("Object = %d (SKY)\n",opmode_var->object);
			printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
			printf("----------------------\n\n");
		}
      	
		usleep(TX_DELAY);
       		log_info("SkyDip data instructions sent to remote server.");
		if (debug_flag) printf("SkyDip data instructions sent to remote server.\n");
		//-------------------------------------------------------

		//CLEAR SERVER_REPLY VARIABLE
                if (debug_flag)
                {
			printf("\n----------------------------------\n");
			printf("Cleanning server_reply variable...\n");
		}
		strncpy(server_reply, "", sizeof(server_reply));
		if (debug_flag)
		{
			printf("Server Reply: [%s]\n", server_reply);
			printf("Char Counter: [%ld]\n", strlen(server_reply));
			printf("----------------------------------\n\n");
		}

		//RECEIVE A REPLY FROM THE SERVER ABOUT VERIFICATION DATA
		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
		{
			log_fatal("Could not receive data from socket server.");
			report_and_exit("Could not receive data from socket server.");
		}
		//-------------------------------------------------------
				
		if (debug_flag) printf("Data received from socket server.\n");
		
		//Displaying information from server
		if (debug_flag) printf("Server Reply: [%s]\n", server_reply);
		if (debug_flag) printf("Char Counter: [%ld]\n", strlen(server_reply));


		// REPLY FROM CONNECT_DATA
		if ( memcmp(server_reply,"NotConnected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope Not Connected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Not Connected.\n\n");
			
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = SKY;
				opmode_var->opmode = STALL;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (SKY)\n",opmode_var->object);
				printf("OpMode = %d (STALL)\n",opmode_var->opmode);
				printf("----------------------\n\n");
       			}
			log_info("Telescope is Tracking off.");
			if (debug_flag || verbose_flag) printf("\rSkyDip Failure. Telescope is Tracking off.\n\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			if (debug_flag) printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			if (debug_flag) printf("File descriptor fp closed.\n");
		
			report_and_exit("Normal exit");
		
		}else if ( memcmp(server_reply,"Parked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Is Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Is Parked.\n\n");
		
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = SKY;
				opmode_var->opmode = STALL;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (SKY)\n",opmode_var->object);
				printf("OpMode = %d (STALL)\n",opmode_var->opmode);
				printf("----------------------\n\n");
	       		}
			log_info("Telescope is Tracking off.");
			if (debug_flag || verbose_flag) printf("\rSkyDip Failure. Telescope is Tracking off.\n\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			if (debug_flag) printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			if (debug_flag) printf("File descriptor fp closed.\n");
	
			report_and_exit("Normal exit");	
		
		}else if ( memcmp(server_reply,"SkyDip.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			// Routine for Op_Mode = SKYDIP
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				//opmode_var->object = SKY;
				opmode_var->opmode = SKY_DIP;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				//printf("Object = %d (SKY)\n",opmode_var->object);
				printf("OpMode = %d (SKY_DIP)\n",opmode_var->opmode);
				printf("----------------------\n\n");
       	 		}
			log_info("Telescope Slewed to position el = %d.", el);
			if (debug_flag) printf("\r\nTelescope Slewed to position el = %d.\n", el);
			if (verbose_flag)  printf("\rTelescope Slewed to position el = %d.\n", el);
			if (debug_flag) printf("\r\nWaiting %d seconds...\n\n", time_dvalue);
			
			if (verbose_flag)  printf("\rWaiting %d seconds...\n\n", time_dvalue);
			sleep(time_dvalue);  //Default value is 10 seconds
			
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");

		}else
		{
			log_info("Telescope Is Parked.");
			if (debug_flag || verbose_flag) printf("\nServer reply not understandable.\n\n");
		
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = SKY;
				opmode_var->opmode = STALL;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (SKY)\n",opmode_var->object);
				printf("OpMode = %d (STALL)\n",opmode_var->opmode);
				printf("----------------------\n\n");
	       		}
			log_info("Telescope is Tracking off.");
			if (debug_flag || verbose_flag) printf("\rSkyDip Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			if (debug_flag) printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			if (debug_flag) printf("File descriptor fp closed.\n");
	
			report_and_exit("Normal exit");		
		}
	}

		
	// Routine for Op_Mode = PARKED
	// - - - - - - - - - - - - - - -
	// THIS IS THE OP_MODE ROUTINE - BEGIN
	// Use semaphore as a mutex (lock) by waiting for writer to increment it
	if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
	{
		opmode_var->object = SKY;
		opmode_var->opmode = STALL;
		sem_post(semptr_opmode);
	}
	// THIS IS THE OP_MODE ROUTINE = END
	// ---------------------------------
	if (debug_flag)
	{
		printf("----------------------\n");
		printf("Operation Mode Set:   \n");
		printf("----------------------\n");
		printf("Object = %d (SKY)\n",opmode_var->object);
		printf("OpMode = %d (STALL)\n",opmode_var->opmode);
		printf("----------------------\n\n");
       		}
	log_info("Telescope is Tracking off.");
	if (debug_flag || verbose_flag) printf("\rSkyDip Done. Telescope is Tracking off.\n\n");
	
	//=================================================================
	close(sock);
	//=================================================================
	log_info("Socket TCP closed.");
	if (debug_flag) printf("Socket TCP closed.\n");
	//Cleanup Op_Mode Shared Memory Stuff
	munmap(opmode_var, ByteSize_opmode);
	close(fd_shmem_opmode);
	sem_close(semptr_opmode);
	if (debug_flag) printf("Shared Memory cleanup ok.\n");
	//---------------------------------------------------------------
	fclose(fp);
	if (debug_flag) printf("File descriptor fp closed.\n");
	//
	if (debug_flag) printf("End of skyDip.\n\n");

	return 0;
}










// Report and Exit  

void report_and_exit(const char * msg)
{
	perror(msg);
	exit(1);
}

// Move string from a to b, according to character x 

void copy_char( char a[], char b[], char x)
{
	int i = 0;
	while ( a[i] != x)
	{
		b[i] = a[i];
		i++;
	}
}


int count_char( char a[], char x)
{
	int i = -1;
	do
	{
		i++;
	}while ( a[i] != x);
	return i;
}


