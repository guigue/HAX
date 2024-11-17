/*
        =================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        =================================================================================

        Scan Target versao 0.6
*/
#define VERSION "0.6"
/*
  	---------------------------------------------------------------------------------
	Este programa executa um escaneamento retilineo sobre um alvo que esta sendo 
	observado e feito o seu acompanhamento. Como premissa, este comando espera que ja
	esteja sendo observado algum objeto, devendo estar o telescopio no modo tracking.
	Quando este comando e executado, inicialmente e feito um offset da posicao atual
	e entao inicia-se o escaneamento a partir do ceu e atravessando o objeto e entao
	finalizando no ceu novamente. Apos realizar N passagens pelo objeto, valor que
	pode ser parametrizado, o telescopio volta a apontar para o objeto e o segue
	acompanhando.	



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
          0.1   |  07-10-2021   | Inicio da programacao com aproveitamento do codigo do
	  	|		| skyDip versao 0.2 e utilizando as opcoes -v, --verbose e 
		|		| --debug para a apresentacao de mensagens na tela. 
        ---------------------------------------------------------------------------------
	  0.2	|  08-10-2021	| Inclusao de variavais com valor inteiro para os  
	  	|		| parametros das opcoes convertidas com atoi(). Versao 
		|		| mais estavel que a 0.1 com a correcao de alguns bugs.
	---------------------------------------------------------------------------------    
	  0.3  	|  13-10-2021	| Mudanca no javascript para ajuste da velocidade de scan 
		|		| e inclusao de tipos de escaneamento como escaneamento
		|		| em RA, DEC, AZ e EL. Devido a uma particularidade do 
		|		| sistema do telescopio, o escaneamento no sistema
		|		| Altazimutal ou horizontal, nao permite uma correcao no
		|		| acompanhamento durante o escaneamento, entao para 
		|		| quantidades acima de 10 passagens ou caso a velocidade
		|		| das passagens seja muito lenta, um objeto como o Sol
		|		| saira facilmente do campo. 
		|		| Implementacao de conversao para que as entradas de
		|		| parametros das opcoes sejam todos em arco minutos.
		|		| Implementacao de mais um script para buscar no servidor
		|		| o valor do tamanho angular dos objetos e condicionar
		|		| outros parametros em funcao deste.
	---------------------------------------------------------------------------------	
	  0.4	|  27-10-2021	| Modificacoes nas linhas a partir da 920 para modificar
	  	|		| informacoes de debug via printf incluindo unidades.
		|		| Modificacoes nas linhas 899 e 908 na equacao, pois o
		|		| correto e offset = (fov - ((fov/2)-(objsize/2))) + offsetScan
		|		| onde offsetScan e a entrada em arcmin, default = 0.
	---------------------------------------------------------------------------------
	  0.5	|  25-02-2022	| Modificacoes na execucao do scan, separando as 
	  	|		| instrucoes javascript para possibilitar a coleta
		|		| das coordenadas pelo GETPOSITION.
		|		| Alteracao nas mensagens de log para permitir visualizar
		|		| os parametros utilizados para configurar o SCAN.
	  	|  12-03-2022	| Varios bugs corrigidos no javascript e na lógica
		|  		| do entorno. Transmissao de loop de scan com apenas
		|		| uma conexao TCP.
	---------------------------------------------------------------------------------
   	  0.6	|  22-03-2022   | Atualiacoes no codigo para ajuste das mensagens de log
		|		|  
	_________________________________________________________________________________


*/

#include <math.h>
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
#include "scanTar.h"	//Definitions for scanTar
#include "opmode.h"	//Definitions for Operation Modes of the Telescope
#include "cfg_buffer.h" //Atualizacao para versao 0.6 - cfgCatcher




#define AccessPermsIN 0644



        /***
        *     P R E L I M I N A R Y     F U N C T I O N S
        *****************************************************************/

void report_and_exit(const char *);
void copy_char ( char a[], char b[], char x);
int count_char ( char a[], char x);
int print_usage();     			// from scanTar.h



	// Tratamento de parametros via getopt()
	// -------------------------------------

//Opt Flag Variables
static int version_flag         ;
static int debug_flag           ;

int nscan_flag		= 0     ;
int offsetScan_flag	= 0	;
int stepSlew_flag	= 0	;
int scanType_flag	= 0	;

int help_flag           = 0     ;
int verbose_flag        = 0     ;

//Opt Arguments
char *nscan_value	= "1"  		;	// Default N times of scan = 1 time
char *offsetScan_value	= "0"		;	// Default offsetScan = 0 arcmin 
char *stepSlew_value	= "1"		;	// Default setpSlew for scan != 1, it is changed below.
char *scanType_value	= "1"		;	// Default is 1 for RA, DEC=2, AZ=3 and EL=4
char *object		= NULL		;	// Not used for scanTar

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
			{"nscan", 	required_argument,	0,	'n'},
			{"offset",  	required_argument,	0,    	'o'},
			{"step",	required_argument,	0,	's'},
			{"type",	required_argument,	0,	't'},
			{"help",        no_argument,		0,      'h'},
			{"verbose",     no_argument,		0,      'v'},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "a:n:o:s:t:hv", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
	
		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
		//		if (memcmp( long_options[option_index].name,"el_init",7)==0)
		//		{
		//			elinit_flag = 1;
		//			optflag_ctr++;
		//			//printf("%s_flag: ", long_options[option_index].name);
		//			if (optarg)
		//			elinit_value = optarg;
		//				//printf("%s\n",elinit_value); }
		//		}
		//		if (memcmp( long_options[option_index].name,"el_end",4)==0)
		//		{
		//			elend_flag = 1;
		//			optflag_ctr++;
		//			//printf("%s_flag: ", long_options[option_index].name);
		//			if (optarg)
		//			elend_value = optarg;
		//				//printf("%s\n",elend_value); }
		//		}
		//		break;
			case 'n':
	                	nscan_flag = 1;
				optflag_ctr++;
				if (optarg) nscan_value = optarg;
				break;
			case 'o':
				offsetScan_flag = 1;
				optflag_ctr++;
				if (optarg) offsetScan_value = optarg;
				break;
			case 's':
				stepSlew_flag = 1;
				optflag_ctr++;
				if (optarg) stepSlew_value = optarg;
				break;
			case 't':
				scanType_flag = 1;
				optflag_ctr++;
				if (optarg) scanType_value = optarg;
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
			printf("scanTar: I don't have any valid arguments, only options. Try -h or --help.\n");
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
		if(nscan_flag) 		printf("nscan_flag = %d\n", nscan_flag);
		if(offsetScan_flag) 	printf("offsetScan_flag = %d\n", offsetScan_flag);
		if(help_flag) 		printf("help_flag = %d\n", help_flag);
		if(verbose_flag) 	printf("verbose_flag = %d\n", verbose_flag);
	}

	
	// Options Errors Verification
	
	if (object && memcmp(object,"-",1) == 0)
	{
		puts("scanTar: Object Sintax Error! Try -h or --help.");
		exit(1);
	}

	if (help_flag == 1 && (argc > 2 || optflag_ctr > 1))
	{
		puts("scanTar: If you need help, don't input more options or parameters! Try -h or --help.");
		exit(1);
	}

	if (version_flag == 1 && argc > 2)
	{
		puts("scanTar: For version information must be used without another option! Try -h or --help.");
		exit(1);
	}
	
//	if (scanType_flag && (memcmp(scanType_value,"1" != 0) || memcmp(scanType_value,"2" != 0) || memcmp(scanType_value,"3" != 0) || memcmp(scanType_value,"4" != 0)  ))
//	{
//		puts("scanTar: Bad parameter for scanType value! Try -h or --help.");
//		exit(1);
//	}

	// The execution starts here, all flags and options is stored
	if (debug_flag) printf("\nRun the scanTar instructions!\n\n");

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





        int i,j					;       //Generic counter

	typedef struct
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
		SCAN_AZ,"SCAN_AZ",	//5
		SCAN_EL,"SCAN_EL",	//6
		SCAN_RA,"SCAN_RA",	//7
		SCAN_DEC,"SCAN_DEC",	//8
		SKY_DIP,"SKY_DIP",      //10
		STALL,"STALL",          //50
		SLEW,"SLEW",            //100
		PARKED,"PARKED",        //150
		OPMODEEND,"OpModeEnd",  //200
	};



        //========================
	// GETTING OPERATION MODE
	for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
	{
		if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
	}
	for (j=0 ; keywordmap[j].targets != EXTRA_OBJ ; j++)    //EXTRA_OBJ = 30
	{
		if ( keywordmap[j].targets == opmode_var->object ) break;
	}
	

	//	printf("----------------------\n");
	//	printf("Operation Mode Status:\n");
	//	printf("----------------------\n");
	//	printf("Object = %d (%s)\n",opmode_var->object, keywordmap[j].keyword);
	//	printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
	//	printf("----------------------\n\n");
	//	report_and_exit("Normal exit");
	


	object = keywordmap[j].keyword;
	
	int objsizeID;

	if ( memcmp(object, "moon", 4) == 0 )
	{
		objsizeID = 125;  //TheSkyX Object Size (Angular Diameter) Code ID for Moon

	}else if ( memcmp(object, "extra object", 12) == 0 )
	{
		log_error("Is not possible to perform the scan for the current object.");
		printf("Is not possible to perform the scan for the current object.\n");
		exit(1);
	}else{
		objsizeID = 120;  //TheSkyX Object Size (Angular Diameter) Code ID for non Moon
	}	



	//=========================
	// Print version code
	if (version_flag)
	{
		printf("-----------------------------------------\n");
		printf("Scan Target - scanTar version %s\n", VERSION);
		printf("-----------------------------------------\n");
		exit(0);
	}



	

	// DECLARACAO VARIAVEIS DA ROTINA DE COMUNICACAO COM SERVIDOR
	// ----------------------------------------------------------
	int 	sock				;       //Socket variable
        struct 	sockaddr_in server		;       //Socket variable
        char 	server_reply[RCV_BUFFER_SIZE]	;       //Socket variable


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
	if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);



	//SENDING DATA INSTRUCTIONS TO SERVER 
	/************************************/
      	//------------------------------------
	
	
	// Javascript instructions for TheSkyX server (scanTar)
	// ------------------------------------------
	char objsize_finder_data[13][50]          ;	// Data to send to server
	int objsize_finder_data_nlines = 13       ;	// Getter commands number of lines from scanTar_data[].
							// If you add a command remember to modify, add, etc a new
							// scanTar_data_nlines
	double	objsize_dvalue;
					
	// BEGIN OF SCANTAR ROUTINE WITH THE SERVER
	// ---------------------------------------

	sprintf(objsize_finder_data[0],"/* Java Script */ 				");
	sprintf(objsize_finder_data[1],"/* Socket Start Packet */ 			");
	sprintf(objsize_finder_data[2],"var Out; 					");
	sprintf(objsize_finder_data[3],"var object = '%s';				", object);
	sprintf(objsize_finder_data[4],"if(!sky6RASCOMTele.IsConnected){ 		");
	sprintf(objsize_finder_data[5],"        Out='NotConnected.'; 			");
	sprintf(objsize_finder_data[6],"}else if(sky6RASCOMTele.IsParked()){ 		");
	sprintf(objsize_finder_data[7],"        Out='Parked.'; 				");
	sprintf(objsize_finder_data[8],"}else{ 						");
	sprintf(objsize_finder_data[9],"sky6StarChart.Find(object);			");
	sprintf(objsize_finder_data[10],"sky6ObjectInformation.Property(%d);		", objsizeID);
	sprintf(objsize_finder_data[11],"Out = sky6ObjectInformation.ObjInfoPropOut;}	");
	sprintf(objsize_finder_data[12],"/* Socket End Packet */			");


	if (debug_flag) printf("\nGetting angular size of object %s, please wait...\n", keywordmap[j].keyword);

	if (verbose_flag) printf("\nGetting angular size of object %s, please wait...", keywordmap[j].keyword);
	fflush(stdout);


	for(i = 0; i<=objsize_finder_data_nlines ; i++)
      	{
		if( send(sock , objsize_finder_data[i] , strlen(objsize_finder_data[i]) , 0) < 0)
		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
		//if (debug_flag) printf("%s\n",objsize_finder_data[i]);
       	}
	//if (debug_flag) printf("\n");

	usleep(TX_DELAY);
       	log_info("Object size finder data instructions sent to remote server.");
	if (debug_flag) printf("Object size finder data instructions sent to remote server.\n");
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

	char objsize_value[strlen(server_reply)];

	// REPLY FROM CONNECT_DATA
	if ( memcmp(server_reply,"NotConnected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
	{
		log_info("Telescope Not Connected.");
		if (debug_flag || verbose_flag) printf("\r\nTelescope Not Connected.\n\n");
			
		// Routine for Op_Mode = STALL
		// - - - - - - - - - - - - - - -
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
		{
			opmode_var->opmode = STALL;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag)
		{
			for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
			{
				if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
			}
			printf("----------------------\n");
			printf("Operation Mode Set:   \n");
			printf("----------------------\n");
			printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
			printf("----------------------\n\n");
		}
		log_info("Telescope is Tracking off.");
		if (debug_flag || verbose_flag) printf("\rScan Target Failure. Telescope is Tracking off.\n\n");
	
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
			//opmode_var->object = SKY;
			opmode_var->opmode = PARKED;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag)
		{
			for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
			{
				if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
			}
			printf("----------------------\n");
			printf("Operation Mode Set:   \n");
			printf("----------------------\n");
			printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
			printf("----------------------\n\n");
		}
		log_info("Telescope is Parked. Tracking off.");
		if (debug_flag || verbose_flag) printf("\rScan Target Failure. Telescope is Parked. Tracking off.\n\n");
	
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
		
	}else 
	{
		if (debug_flag || verbose_flag) printf("Done!\n\n");
		copy_char( server_reply, objsize_value, '|');
		if (debug_flag) printf("Angular Size of %s = [%s] string.\n", object, objsize_value);
		objsize_dvalue = atof(objsize_value);
		if (debug_flag) printf("Angular Size of %s = [%f] float.\n", object, objsize_dvalue);
	
		//=================================================================
	//	close(sock);
		//=================================================================
	//	log_info("Socket TCP closed.");
	//	if (debug_flag) printf("Socket TCP closed.\n\n");
	}
	



	


	// CONVERTION OF STRING TO THEIR RELATIVE DECIMALS
	// -----------------------------------------------
	int 	nscan_dvalue			;
	double 	offsetScan_dvalue		;
	double 	stepSlew_dvalue			;
	int 	scanType_dvalue			;

	nscan_dvalue = atoi(nscan_value);
	//if (debug_flag) printf("nscan_dvalue = %d [times]\n", nscan_dvalue);

	offsetScan_dvalue = atof(offsetScan_value);
	//if (debug_flag) printf("offsetScan_dvalue = %f [arcmin]\n", offsetScan_dvalue);

	stepSlew_dvalue = atof(stepSlew_value);
	//if (debug_flag) printf("stepSlew_dvalue = %f [arcmin]\n", stepSlew_dvalue);

	scanType_dvalue = atoi(scanType_value);
	//if (debug_flag) printf("scanType_dvalue = %d [type: RA=1, DEC=2, AZ=3, EL=4]\n", scanType_dvalue);


	if (verbose_flag || debug_flag) printf("-------------------------------------------\n");
	if (verbose_flag || debug_flag) printf("ScanTar Parameters:\n");
	if (verbose_flag || debug_flag) printf("-------------------------------------------\n");
	if (verbose_flag || debug_flag) printf("Object = %s\n", object);
	if (verbose_flag || debug_flag) printf("Nscan  = %d times\n", nscan_dvalue);
	if (verbose_flag || debug_flag) printf("Offset = %f [arcmin]\n", offsetScan_dvalue);
	if (verbose_flag || debug_flag) printf("Step   = %f [arcmin]\n", stepSlew_dvalue);
	if (verbose_flag || debug_flag) printf("Type   = %d [type: RA=1, Dec=2, Az=3, El=4]\n", scanType_dvalue);
	if (verbose_flag || debug_flag) printf("-------------------------------------------\n");


	log_info("-------------------------------------------");
	log_info("ScanTar Parameters:");
	log_info("-------------------------------------------");
	log_info("Object = %s", object);
	log_info("Nscan  = %d times", nscan_dvalue);
	log_info("Offset = %f [arcmin]", offsetScan_dvalue);
	log_info("Step	 = %f [arcmin]", stepSlew_dvalue);
	log_info("Type   = %d [type: RA=1, Dec=2, Az=3, El=4]", scanType_dvalue);
	log_info("-------------------------------------------");

	
	// COORDINATES UNITS CONVERTION
	// ============================

	// HATS objetive diameter = 457 mm
	// HATS focal length (f) = 1007 mm
	// Solar disc angular size (theta) = 0.5 degrees = 30 arcmin = 1800 arcsec
	// Solar image size = f.tan(theta) = 8.8 mm
	// HATS detector apperture = 10 mm
	//
	// So.. if 8.8 mm ----> 30 arcmin, then 10 mm ----> 34 arcmin (for the Field of View)
	
	double fov = 34;			// Field of View for HATS Telescope [arcmin]	
	
	double offset;
	double objsize = objsize_dvalue*60;	//Convertion from degree to arcmin
	

	if (scanType_dvalue == 1)
	{
		offset = ((((fov - ((fov/2)-(objsize/2))) + offsetScan_dvalue )/60 )/15 );	//Decimal Sexagenal Hour 
	
		if (stepSlew_flag)
		{
			stepSlew_dvalue = ((stepSlew_dvalue/60)/15);			//Decimal Sexagenal Hour (custom)
		}else{
			stepSlew_dvalue = (2*offset)/20;				//Degree 
											// 1/20 of scan course (default)
		}
	}else{
		offset = (((fov - ((fov/2)-(objsize/2))) + offsetScan_dvalue )/60 ); 	//Degree
		
		if (stepSlew_flag)
		{
			stepSlew_dvalue = (stepSlew_dvalue/60);				//Degree (custom)
		}else{
			stepSlew_dvalue = ((2*offset)/20);				//Degree
											// 1/20 of scan course (default)
		}	
	}



	if (scanType_dvalue == 1)
	{
		if (debug_flag) printf("\n------------------------------\n");
		if (debug_flag) printf("After convertion calculations:\n");
		if (debug_flag) printf("------------------------------\n");
		if (debug_flag) printf("offset = %f [Decimal Sexagenal Hour]\n", offset);
		if (debug_flag) printf("stepSlew_dvalue = %f [Decimal Sexagenal Hour]\n", stepSlew_dvalue);
		if (debug_flag) printf("------------------------------\n\n");
	}else{
		if (debug_flag) printf("\n------------------------------\n");
		if (debug_flag) printf("After convertion calculations:\n");
		if (debug_flag) printf("------------------------------\n");
		if (debug_flag) printf("offset = %f [degree]\n", offset);
		if (debug_flag) printf("stepSlew_dvalue = %f [degree]\n", stepSlew_dvalue);
		if (debug_flag) printf("------------------------------\n\n");
	}


	log_info("Size of %s = %d [arcmin]", object,objsize);
	log_info("HATS FOV = %f [arcmin]", fov);
	



	// BEGIN OF SCANTAR ROUTINE WITH THE SERVER
	// ---------------------------------------
	// * * * * * * * * * * * * * * * * * * * * 



/*	// TCP Communication
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
	if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);
*/


	
	//======================================
	// Getting Start Coordinate
	//======================================

	//Rotina Javascript para coleta das coordenadas iniciais do objeto
	//----------------------------------------------------------------
	char scanTar_data_1[14][150]          ;	// Data to send to server
	int scanTar_data_1_nlines = 14        ;	// Getter commands number of lines from scanTar_data[].
	char * p;
	const char sep[2] = ";";
	
	double ra, dec, az, el;

	sprintf(scanTar_data_1[0],"/* Java Script */ ");
	sprintf(scanTar_data_1[1],"/* Socket Start Packet */ ");
	sprintf(scanTar_data_1[2],"var Out; ");
	sprintf(scanTar_data_1[3],"var object = '%s';    ", object);
	sprintf(scanTar_data_1[4],"sky6StarChart.Find(object); ");
	sprintf(scanTar_data_1[5],"sky6ObjectInformation.Property(54); ");
	sprintf(scanTar_data_1[6],"  var ra = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_1[7],"sky6ObjectInformation.Property(55); ");
	sprintf(scanTar_data_1[8],"  var dec = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_1[9],"sky6ObjectInformation.Property(58); ");
	sprintf(scanTar_data_1[10],"  var az = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_1[11],"sky6ObjectInformation.Property(59); ");
	sprintf(scanTar_data_1[12],"  var el = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_1[13],"Out = ra + ';' + dec + ';' + az + ';' + el;  ");
	sprintf(scanTar_data_1[14],"/* Socket End Packet */ ");


	//SENDING DATA INSTRUCTIONS TO SERVER 
	/************************************/
      	//-----To get the coordenate to perform the scan------
	
	if (debug_flag) printf("\n");
	/************************************/
      	//-----To get the coordenate to perform the scan------
	
	if (debug_flag) printf("\n");
	for(i = 0; i<=scanTar_data_1_nlines ; i++)
      	{
		if( send(sock , scanTar_data_1[i] , strlen(scanTar_data_1[i]) , 0) < 0)
		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
		if (debug_flag) printf("%s\n",scanTar_data_1[i]);
       	}
	if (debug_flag) printf("\n");

	//Receiving data from server - inicial procedures
	usleep(TX_DELAY);
       	log_info("Scan Target data instructions sent to remote server.");
	if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");

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

	//=================================================================
	close(sock);
	//=================================================================
	log_info("Socket TCP closed.");
	if (debug_flag) printf("Socket TCP closed.\n\n");
	
	
	//Extracting data from server reply in ';' symbol steps
	//-----------------------------------------------------
	
	p = strtok(server_reply,sep);
	
	if ( p != NULL )
	{
		ra = atof(p);
		p = strtok(NULL,sep);
	}
	if ( p != NULL )
	{
		dec = atof(p);
		p = strtok(NULL,sep);
	}
	if ( p != NULL )
	{
		az = atof(p);
		p = strtok(NULL,sep);
	}
	if ( p != NULL )
	{
		el = atof(p);
		p = strtok(NULL,sep);
	}



	//============================
	// OFFSET CALCULATION
	//============================

	double offset1_ra, offset2_ra, offset1_dec, offset2_dec;
	double offset1_az, offset2_az, offset1_el, offset2_el;
	int 	subscan_dvalue			;	// Counter for N step slews for each scan direction


	offset1_ra = ra + offset;
	offset2_ra = ra - offset;
	offset1_dec = dec + offset;
	offset2_dec = dec - offset;
	offset1_az = az + offset;
	offset2_az = az - offset;
	offset1_el = el + offset;
	offset2_el = el - offset;

  	
	if (scanType_dvalue == 1) subscan_dvalue = ceil((offset1_ra - offset2_ra)/stepSlew_dvalue);
	if (scanType_dvalue == 2) subscan_dvalue = ceil((offset1_dec - offset2_dec)/stepSlew_dvalue);
	if (scanType_dvalue == 3) subscan_dvalue = ceil((offset1_az - offset2_az)/stepSlew_dvalue);
	if (scanType_dvalue == 4) subscan_dvalue = ceil((offset1_el - offset2_el)/stepSlew_dvalue);

	
	
	//========================================
	// Slew to Offset Position 
	//========================================


	//Rotina Javascript para SLEW até coordenada offset - coleta das coordenadas iniciais do objeto
	//---------------------------------------------------------------------------------------------
	char scanTar_data_2[21][150]          ;	// Data to send to server
	int scanTar_data_2_nlines = 21        ;	// Getter commands number of lines from scanTar_data[].
	
	sprintf(scanTar_data_2[0],"/* Java Script */ ");
	sprintf(scanTar_data_2[1],"/* Socket Start Packet */ ");
	sprintf(scanTar_data_2[2],"var Out,SlewComplete; ");
	sprintf(scanTar_data_2[3],"var object = '%s'; "		,object);
	sprintf(scanTar_data_2[4],"var scanType = %d; "		,scanType_dvalue);
	sprintf(scanTar_data_2[5],"var ra = %f; "		,ra);
	sprintf(scanTar_data_2[6],"var dec = %f; "		,dec);
	sprintf(scanTar_data_2[7],"var az = %f; "		,az);
	sprintf(scanTar_data_2[8],"var el = %f; "		,el);
	sprintf(scanTar_data_2[9],"var offset1_ra = %f; "	,offset1_ra);
	sprintf(scanTar_data_2[10],"var offset1_dec = %f; "	,offset1_dec);
	sprintf(scanTar_data_2[11],"var offset1_az = %f; "	,offset1_az);
	sprintf(scanTar_data_2[12],"var offset1_el = %f; "	,offset1_el);
	sprintf(scanTar_data_2[13],"sky6StarChart.Find(object); ");
	sprintf(scanTar_data_2[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset1_ra, dec, object);} ");
	sprintf(scanTar_data_2[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset1_dec, object);} ");
	sprintf(scanTar_data_2[16],"if (scanType == 3) {sky6RASCOMTele.SlewToAzAlt(offset1_az, el, object); } ");
	sprintf(scanTar_data_2[17],"if (scanType == 4) {sky6RASCOMTele.SlewToAzAlt(az, offset1_el, object); } ");
	sprintf(scanTar_data_2[18],"while(SlewComplete != 1){ ");
	sprintf(scanTar_data_2[19],"       SlewComplete = sky6RASCOMTele.IsSlewComplete;} ");
	sprintf(scanTar_data_2[20],"Out = 'OffsetDone.'; ");
	sprintf(scanTar_data_2[21],"/* Socket End Packet */ ");


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
	if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);



	// Routine for Op_Mode 
	// - - - - - - - - - - - - - - -
	// THIS IS THE OP_MODE ROUTINE - BEGIN
	// Use semaphore as a mutex (lock) by waiting for writer to increment it
	if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
	{
		opmode_var->opmode = SLEW;
		sem_post(semptr_opmode);
	}
	// THIS IS THE OP_MODE ROUTINE = END
	// ---------------------------------
	if (debug_flag)
	{
		for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
		{
			if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
		}
		printf("\n----------------------\n");
		printf("Operation Mode Set:   \n");
		printf("----------------------\n");
		printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
		printf("----------------------\n\n");
	}
	

	//SENDING DATA INSTRUCTIONS TO SERVER 
	/************************************/
      	//-----Move to offset position - Begin of SCAN ------
	
	if (debug_flag) printf("\n");
	for(i = 0; i<=scanTar_data_2_nlines ; i++)
      	{
		if( send(sock , scanTar_data_2[i] , strlen(scanTar_data_2[i]) , 0) < 0)
		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
		if (debug_flag) printf("%s\n",scanTar_data_2[i]);
       		usleep(50000);
	}
	if (debug_flag) printf("\n");

	//Receiving data from server - inicial procedures
	usleep(TX_DELAY);

       	log_info("Scan Target data instructions sent to remote server.");
	if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");

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

	//=================================================================
	close(sock);
	//=================================================================
	log_info("Socket TCP closed.");
	if (debug_flag) printf("Socket TCP closed.\n\n");
	
	// VERIFYING IF SO FAR SO GOOD - for Offset Position
	if ( memcmp(server_reply,"OffsetDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		if (verbose_flag) printf("\nOffset Position Done. Ready to start the Scan.\n\n");

		// =========================================
		// CONDITIONAL ATRIBUTION FOR OPERATION MODE
		// =========================================
		if (scanType_dvalue == 1)
		{
			// Routine for Op_Mode 
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->opmode = SCAN_RA;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
				{
					if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
				}
				printf("\n----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
				printf("----------------------\n\n");
			}
		}
      		if (scanType_dvalue == 2)
		{
			// Routine for Op_Mode 
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->opmode = SCAN_DEC;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
				{
					if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
				}
				printf("\n----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
				printf("----------------------\n\n");
			}
		}
		if (scanType_dvalue == 3)
		{
			// Routine for Op_Mode 
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->opmode = SCAN_AZ;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
				{
					if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
				}
				printf("\n----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
				printf("----------------------\n\n");
			}
		}
		if (scanType_dvalue == 4)
		{
			// Routine for Op_Mode 
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->opmode = SCAN_EL;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
				{
					if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
				}
				printf("\n----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
				printf("----------------------\n\n");
			}
		}
	}else
	{
		log_info("Server reply not understandable.");
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
		if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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

	
	//========================================
	// Performing the SCAN - Beginning of scan
	//========================================


	//Print beginning information of the scan
	if( nscan_dvalue == 1 )
	{
		if (debug_flag || verbose_flag) printf("Performing one scan of %s, please wait...\n", object);
		sleep(5);
		//fflush(stdout);
	}else
	{
		if (debug_flag || verbose_flag) printf("Performing scan of %s %d times, please wait...\n", object, nscan_dvalue);
		sleep(5);
		//fflush(stdout);
	}

	int k;
	int subscan_var = 1;

	for (k = 0 ; k < nscan_dvalue ; k++ )
	{

		if (scanType_dvalue == 1)
		{


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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------

			for (offset1_ra ; offset1_ra >= offset2_ra ; offset1_ra = offset1_ra - stepSlew_dvalue )
			{
			
				if(verbose_flag) system("clear"); // Clean Screen

				if (verbose_flag) printf("\nGetting angular size of object %s, please wait...Done!\n\n", keywordmap[j].keyword);
		
				if (verbose_flag) printf("-------------------------------------------\n");
				if (verbose_flag) printf("ScanTar Parameters:\n");
				if (verbose_flag) printf("-------------------------------------------\n");
				if (verbose_flag) printf("Object = %s\n", object);
				if (verbose_flag) printf("Nscan  = %d times\n", nscan_dvalue);
				if (verbose_flag) printf("Offset = %f [arcmin]\n", offsetScan_dvalue);
				if (verbose_flag) printf("Step   = %f [arcmin]\n", stepSlew_dvalue);
				if (verbose_flag) printf("Type   = %d [type: RA=1, Dec=2, Az=3, El=4]\n", scanType_dvalue);
				if (verbose_flag) printf("-------------------------------------------\n");

				if (verbose_flag) printf("\nOffset Position Done. Ready to start the Scan.\n\n");

				//Print beginning information of the scan
				if( nscan_dvalue == 1 )
				{
					if (debug_flag || verbose_flag) printf("Performing one scan of %s, please wait...\n", object);
					fflush(stdout);
				}else
				{
					if (debug_flag || verbose_flag) printf("Performing scan of %s %d times, please wait...\n", object, nscan_dvalue);
					fflush(stdout);
				}
	
				if (debug_flag || verbose_flag) printf("\noffset1_ra = %f\n",offset1_ra);
				if (debug_flag || verbose_flag) printf("offset2_ra = %f\n",offset2_ra);
				if (debug_flag || verbose_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag || verbose_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag || verbose_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);
				fflush(stdout);		
				subscan_var++;
				
				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_3[27][150]          ;	// Data to send to server
				int scanTar_data_3_nlines = 27        ;	// Getter commands number 
									//of lines from scanTar_data[].
	
		sprintf(scanTar_data_3[0],"/* Java Script */ ");
		sprintf(scanTar_data_3[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_3[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_3[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_3[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_3[5],"var ra = %f; "		,ra);
		sprintf(scanTar_data_3[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_3[7],"var az = %f; "		,az);
		sprintf(scanTar_data_3[8],"var el = %f; "		,el);
		sprintf(scanTar_data_3[9],"var offset1_ra = %f; "	,offset1_ra);
		sprintf(scanTar_data_3[10],"var offset1_dec = %f; "	,offset1_dec);
		sprintf(scanTar_data_3[11],"var offset1_az = %f; "	,offset1_az);
		sprintf(scanTar_data_3[12],"var offset1_el = %f; "	,offset1_el);
		sprintf(scanTar_data_3[13],"sky6StarChart.Find(object);");
		sprintf(scanTar_data_3[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset1_ra, dec, object);} ");
		sprintf(scanTar_data_3[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset1_dec, object);} ");
		sprintf(scanTar_data_3[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_3[17],"			sky6ObjectInformation.Property(58);                   ");
		sprintf(scanTar_data_3[18],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[19],"			sky6RASCOMTele.SlewToAzAlt(offset1_az, el, object); } ");
		sprintf(scanTar_data_3[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_3[21],"			sky6ObjectInformation.Property(59);		      ");
		sprintf(scanTar_data_3[22],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset1_el, object); } ");
		sprintf(scanTar_data_3[24],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_3[25],"       SlewComplete = sky6RASCOMTele.IsSlewComplete;} ");
		sprintf(scanTar_data_3[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_3[27],"/* Socket End Packet */ ");

					


				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_3_nlines ; i++)
				{
					if( send(sock , scanTar_data_3[i] , strlen(scanTar_data_3[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_3[i]);
					usleep(5000);
				}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);

				if (debug_flag) printf("TX_DELAY=%ld\n",TX_DELAY);

			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
			//	close(sock);
				//=================================================================
			//	log_info("Socket TCP closed.");
			//	if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{
		
					//if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");

				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
		
		}
		if (scanType_dvalue == 2)
		{
	
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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------

			for (offset1_dec ; offset1_dec >= offset2_dec ; offset1_dec = offset1_dec - stepSlew_dvalue )
			{
	
				if (debug_flag) printf("\noffset1_dec = %f\n",offset1_dec);
				if (debug_flag) printf("offset2_dec = %f\n",offset2_dec);
				if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);
	
				subscan_var++;
					

				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_3[27][150]          ;	// Data to send to server
				int scanTar_data_3_nlines = 27        ;	// Getter commands number 
									//of lines from scanTar_data[].
	
		sprintf(scanTar_data_3[0],"/* Java Script */ ");
		sprintf(scanTar_data_3[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_3[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_3[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_3[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_3[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_3[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_3[7],"var az = %f; "		,az);
		sprintf(scanTar_data_3[8],"var el = %f; "		,el);
		sprintf(scanTar_data_3[9],"var offset1_ra = %f; "	,offset1_ra);
		sprintf(scanTar_data_3[10],"var offset1_dec = %f; "	,offset1_dec);
		sprintf(scanTar_data_3[11],"var offset1_az = %f; "	,offset1_az);
		sprintf(scanTar_data_3[12],"var offset1_el = %f; "	,offset1_el);
		sprintf(scanTar_data_3[13],"sky6StarChart.Find(object);");
		sprintf(scanTar_data_3[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset1_ra, dec, object);} ");
		sprintf(scanTar_data_3[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset1_dec, object);} ");
		sprintf(scanTar_data_3[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_3[17],"			sky6ObjectInformation.Property(58);                   ");
		sprintf(scanTar_data_3[18],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[19],"			sky6RASCOMTele.SlewToAzAlt(offset1_az, el, object); } ");
		sprintf(scanTar_data_3[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_3[21],"			sky6ObjectInformation.Property(59);		      ");
		sprintf(scanTar_data_3[22],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset1_el, object); } ");
		sprintf(scanTar_data_3[24],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_3[25],"      SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_3[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_3[27],"/* Socket End Packet */ ");

		

				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_3_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_3[i] , strlen(scanTar_data_3[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_3[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{
		
					if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");
		
				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
		
		}			
		if (scanType_dvalue == 3)
		{
		
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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------

			for (offset1_az ; offset1_az >= offset2_az ; offset1_az = offset1_az - stepSlew_dvalue )
			{
		
				if (debug_flag) printf("\noffset1_az = %f\n",offset1_az);
				if (debug_flag) printf("offset2_az = %f\n",offset2_az);
				if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);
	
				subscan_var++;
		
				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_3[27][150]          ;	// Data to send to server
				int scanTar_data_3_nlines = 27        ;	// Getter commands number 
									//of lines from scanTar_data[].
	
		sprintf(scanTar_data_3[0],"/* Java Script */ ");
		sprintf(scanTar_data_3[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_3[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_3[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_3[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_3[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_3[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_3[7],"var az = %f; "		,az);
		sprintf(scanTar_data_3[8],"var el = %f; "		,el);
		sprintf(scanTar_data_3[9],"var offset1_ra = %f; "	,offset1_ra);
		sprintf(scanTar_data_3[10],"var offset1_dec = %f; "	,offset1_dec);
		sprintf(scanTar_data_3[11],"var offset1_az = %f; "	,offset1_az);
		sprintf(scanTar_data_3[12],"var offset1_el = %f; "	,offset1_el);
		sprintf(scanTar_data_3[13],"sky6StarChart.Find(object);");
		sprintf(scanTar_data_3[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset1_ra, dec, object);} ");
		sprintf(scanTar_data_3[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset1_dec, object);} ");
		sprintf(scanTar_data_3[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_3[17],"			sky6ObjectInformation.Property(59);                   ");
		sprintf(scanTar_data_3[18],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[19],"			sky6RASCOMTele.SlewToAzAlt(offset1_az, el, object); } ");
		sprintf(scanTar_data_3[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_3[21],"			sky6ObjectInformation.Property(58);		      ");
		sprintf(scanTar_data_3[22],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset1_el, object); } ");
		sprintf(scanTar_data_3[24],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_3[25],"         SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_3[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_3[27],"/* Socket End Packet */ ");



				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_3_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_3[i] , strlen(scanTar_data_3[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_3[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{
		
					if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");
		
				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
	
		}
		if (scanType_dvalue == 4)
		{
		
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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------

			for (offset1_el ; offset1_el >= offset2_el ; offset1_el = offset1_el - stepSlew_dvalue )
			{
		
				if (debug_flag) printf("\noffset1_el = %f\n",offset1_el);
				if (debug_flag) printf("offset2_el = %f\n",offset2_el);
				if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);
	
				subscan_var++;
		
				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_3[27][150]          ;	// Data to send to server
				int scanTar_data_3_nlines = 27        ;	// Getter commands number 
									//of lines from scanTar_data[].
	
		sprintf(scanTar_data_3[0],"/* Java Script */ ");
		sprintf(scanTar_data_3[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_3[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_3[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_3[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_3[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_3[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_3[7],"var az = %f; "		,az);
		sprintf(scanTar_data_3[8],"var el = %f; "		,el);
		sprintf(scanTar_data_3[9],"var offset1_ra = %f; "	,offset1_ra);
		sprintf(scanTar_data_3[10],"var offset1_dec = %f; "	,offset1_dec);
		sprintf(scanTar_data_3[11],"var offset1_az = %f; "	,offset1_az);
		sprintf(scanTar_data_3[12],"var offset1_el = %f; "	,offset1_el);
		sprintf(scanTar_data_3[13],"sky6StarChart.Find(object);");
		sprintf(scanTar_data_3[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset1_ra, dec, object);} ");
		sprintf(scanTar_data_3[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset1_dec, object);} ");
		sprintf(scanTar_data_3[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_3[17],"			sky6ObjectInformation.Property(59);                   ");
		sprintf(scanTar_data_3[18],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[19],"			sky6RASCOMTele.SlewToAzAlt(offset1_az, el, object); } ");
		sprintf(scanTar_data_3[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_3[21],"			sky6ObjectInformation.Property(58);		      ");
		sprintf(scanTar_data_3[22],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_3[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset1_el, object); } ");
		sprintf(scanTar_data_3[24],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_3[25],"         SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_3[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_3[27],"/* Socket End Packet */ ");


				
				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_3_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_3[i] , strlen(scanTar_data_3[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_3[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{

					if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");

				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");

		}
		
		k++;

		if ( k == nscan_dvalue ) break;   
	
		subscan_var = 1;

		offset1_ra = ra + offset;
		offset2_ra = ra - offset;
		offset1_dec = dec + offset;
		offset2_dec = dec - offset;
		offset1_az = az + offset;
		offset2_az = az - offset;
		offset1_el = el + offset;
		offset2_el = el - offset;	

		
		if (scanType_dvalue == 1)
		{

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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------

			for (offset2_ra ; offset2_ra <= offset1_ra ; offset2_ra = offset2_ra + stepSlew_dvalue )
			{
	
				if (verbose_flag) printf("\nGetting angular size of object %s, please wait...Done!\n\n", keywordmap[j].keyword);
		
				if (verbose_flag) printf("-------------------------------------------\n");
				if (verbose_flag) printf("ScanTar Parameters:\n");
				if (verbose_flag) printf("-------------------------------------------\n");
				if (verbose_flag) printf("Object = %s\n", object);
				if (verbose_flag) printf("Nscan  = %d times\n", nscan_dvalue);
				if (verbose_flag) printf("Offset = %f [arcmin]\n", offsetScan_dvalue);
				if (verbose_flag) printf("Step   = %f [arcmin]\n", stepSlew_dvalue);
				if (verbose_flag) printf("Type   = %d [type: RA=1, Dec=2, Az=3, El=4]\n", scanType_dvalue);
				if (verbose_flag) printf("-------------------------------------------\n");

				if (verbose_flag) printf("\nOffset Position Done. Ready to start the Scan.\n\n");

				//Print beginning information of the scan
				if( nscan_dvalue == 1 )
				{
					if (debug_flag || verbose_flag) printf("Performing one scan of %s, please wait...\n", object);
					fflush(stdout);
				}else
				{
					if (debug_flag || verbose_flag) printf("Performing scan of %s %d times, please wait...\n", object, nscan_dvalue);
					fflush(stdout);
				}
	
				if (debug_flag || verbose_flag) printf("\noffset1_ra = %f\n",offset1_ra);
				if (debug_flag || verbose_flag) printf("offset2_ra = %f\n",offset2_ra);
				if (debug_flag || verbose_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag || verbose_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag || verbose_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);
				fflush(stdout);		
				subscan_var++;
				
				//if (debug_flag) printf("\noffset1_ra = %f\n",offset1_ra);
				//if (debug_flag) printf("offset2_ra = %f\n",offset2_ra);
				//if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				//if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				//if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);				
				//subscan_var++;
		

				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_4[27][150]          ;	// Data to send to server
				int scanTar_data_4_nlines = 27        ;	// Getter commands number 
									// of lines from scanTar_data[].
	
		sprintf(scanTar_data_4[0],"/* Java Script */ ");
		sprintf(scanTar_data_4[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_4[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_4[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_4[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_4[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_4[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_4[7],"var az = %f; "		,az);
		sprintf(scanTar_data_4[8],"var el = %f; "		,el);
		sprintf(scanTar_data_4[9],"var offset2_ra = %f; "	,offset2_ra);
		sprintf(scanTar_data_4[10],"var offset2_dec = %f; "	,offset2_dec);
		sprintf(scanTar_data_4[11],"var offset2_az = %f; "	,offset2_az);
		sprintf(scanTar_data_4[12],"var offset2_el = %f; "	,offset2_el);
		sprintf(scanTar_data_4[13],"sky6StarChart.Find(object); ");
		sprintf(scanTar_data_4[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset2_ra, dec, object);} ");
		sprintf(scanTar_data_4[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset2_dec, object);} ");
		sprintf(scanTar_data_4[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_4[17],"			sky6ObjectInformation.Property(58);                   ");
		sprintf(scanTar_data_4[18],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[19],"			sky6RASCOMTele.SlewToAzAlt(offset2_az, el, object); } ");
		sprintf(scanTar_data_4[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_4[21],"			sky6ObjectInformation.Property(59);		      ");
		sprintf(scanTar_data_4[22],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset2_el, object); } ");
		sprintf(scanTar_data_4[34],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_4[25],"         SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_4[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_4[27],"/* Socket End Packet */ ");

					

				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_4_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_4[i] , strlen(scanTar_data_4[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_4[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{
			
					//if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");
			
				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
	
		}
		if (scanType_dvalue == 2)
		{
			
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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------
			
			for (offset2_dec ; offset2_dec <= offset1_dec ; offset2_dec = offset2_dec + stepSlew_dvalue )
			{
			
				if (debug_flag) printf("\noffset1_dec = %f\n",offset1_dec);
				if (debug_flag) printf("offset2_dec = %f\n",offset2_dec);
				if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);				
				subscan_var++;
		
				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_4[27][150]          ;	// Data to send to server
				int scanTar_data_4_nlines = 27        ;	// Getter commands number 
									// of lines from scanTar_data[].
	
		sprintf(scanTar_data_4[0],"/* Java Script */ ");
		sprintf(scanTar_data_4[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_4[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_4[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_4[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_4[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_4[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_4[7],"var az = %f; "		,az);
		sprintf(scanTar_data_4[8],"var el = %f; "		,el);
		sprintf(scanTar_data_4[9],"var offset2_ra = %f; "	,offset2_ra);
		sprintf(scanTar_data_4[10],"var offset2_dec = %f; "	,offset2_dec);
		sprintf(scanTar_data_4[11],"var offset2_az = %f; "	,offset2_az);
		sprintf(scanTar_data_4[12],"var offset2_el = %f; "	,offset2_el);
		sprintf(scanTar_data_4[13],"sky6StarChart.Find(object); ");
		sprintf(scanTar_data_4[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset2_ra, dec, object);} ");
		sprintf(scanTar_data_4[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset2_dec, object);} ");
		sprintf(scanTar_data_4[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_4[17],"			sky6ObjectInformation.Property(58);                   ");
		sprintf(scanTar_data_4[18],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[19],"			sky6RASCOMTele.SlewToAzAlt(offset2_az, el, object); } ");
		sprintf(scanTar_data_4[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_4[21],"			sky6ObjectInformation.Property(59);		      ");
		sprintf(scanTar_data_4[22],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset2_el, object); } ");
		sprintf(scanTar_data_4[34],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_4[25],"         SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_4[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_4[27],"/* Socket End Packet */ ");

				

				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_4_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_4[i] , strlen(scanTar_data_4[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_4[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{

					if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");
	
				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
			
		}
		if (scanType_dvalue == 3)
		{
	
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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------
		
			for (offset2_az ; offset2_az <= offset1_az ; offset2_az = offset2_az + stepSlew_dvalue )
			{
	
				if (debug_flag) printf("\noffset1_az = %f\n",offset1_az);
				if (debug_flag) printf("offset2_az = %f\n",offset2_az);
				if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);				
				subscan_var++;
		
				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_4[27][150]          ;	// Data to send to server
				int scanTar_data_4_nlines = 27        ;	// Getter commands number 
									// of lines from scanTar_data[].
	
		sprintf(scanTar_data_4[0],"/* Java Script */ ");
		sprintf(scanTar_data_4[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_4[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_4[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_4[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_4[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_4[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_4[7],"var az = %f; "		,az);
		sprintf(scanTar_data_4[8],"var el = %f; "		,el);
		sprintf(scanTar_data_4[9],"var offset2_ra = %f; "	,offset2_ra);
		sprintf(scanTar_data_4[10],"var offset2_dec = %f; "	,offset2_dec);
		sprintf(scanTar_data_4[11],"var offset2_az = %f; "	,offset2_az);
		sprintf(scanTar_data_4[12],"var offset2_el = %f; "	,offset2_el);
		sprintf(scanTar_data_4[13],"sky6StarChart.Find(object); ");
		sprintf(scanTar_data_4[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset2_ra, dec, object);} ");
		sprintf(scanTar_data_4[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset2_dec, object);} ");
		sprintf(scanTar_data_4[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_4[17],"			sky6ObjectInformation.Property(58);                   ");
		sprintf(scanTar_data_4[18],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[19],"			sky6RASCOMTele.SlewToAzAlt(offset2_az, el, object); } ");
		sprintf(scanTar_data_4[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_4[21],"			sky6ObjectInformation.Property(59);		      ");
		sprintf(scanTar_data_4[22],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset2_el, object); } ");
		sprintf(scanTar_data_4[34],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_4[25],"         SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_4[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_4[27],"/* Socket End Packet */ ");



				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_4_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_4[i] , strlen(scanTar_data_4[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_4[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{

					if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");

				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
				
		}
		if (scanType_dvalue == 4)
		{
		
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
			if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


			// SCAN LOOP 
			// ---------
		
			for (offset2_el ; offset2_el <= offset1_el ; offset2_el = offset2_el + stepSlew_dvalue )
			{
		
				if (debug_flag) printf("\noffset1_el = %f\n",offset1_el);
				if (debug_flag) printf("offset2_el = %f\n",offset2_el);
				if (debug_flag) printf("stepSlew   = %f\n",stepSlew_dvalue);
				if (debug_flag) printf("NScan   = %d/%d\n",k+1,nscan_dvalue);
				if (debug_flag) printf("SubScan = %d/%d\n\n",subscan_var,subscan_dvalue);				
				subscan_var++;

				// Rotina Javascript para realizacao do SCAN - Step-by-Step Slew.
				// --------------------------------------------------------------
				char scanTar_data_4[27][150]          ;	// Data to send to server
				int scanTar_data_4_nlines = 27        ;	// Getter commands number 
									// of lines from scanTar_data[].
	
		sprintf(scanTar_data_4[0],"/* Java Script */ ");
		sprintf(scanTar_data_4[1],"/* Socket Start Packet */ ");
		sprintf(scanTar_data_4[2],"var Out,SlewComplete; ");
		sprintf(scanTar_data_4[3],"var object = '%s';    "	,object);
		sprintf(scanTar_data_4[4],"var scanType = %d;    "	,scanType_dvalue);
		sprintf(scanTar_data_4[5],"var ra= %f; "		,ra);
		sprintf(scanTar_data_4[6],"var dec = %f; "		,dec);
		sprintf(scanTar_data_4[7],"var az = %f; "		,az);
		sprintf(scanTar_data_4[8],"var el = %f; "		,el);
		sprintf(scanTar_data_4[9],"var offset2_ra = %f; "	,offset2_ra);
		sprintf(scanTar_data_4[10],"var offset2_dec = %f; "	,offset2_dec);
		sprintf(scanTar_data_4[11],"var offset2_az = %f; "	,offset2_az);
		sprintf(scanTar_data_4[12],"var offset2_el = %f; "	,offset2_el);
		sprintf(scanTar_data_4[13],"sky6StarChart.Find(object); ");
		sprintf(scanTar_data_4[14],"if (scanType == 1) {sky6RASCOMTele.SlewToRaDec(offset2_ra, dec, object);} ");
		sprintf(scanTar_data_4[15],"if (scanType == 2) {sky6RASCOMTele.SlewToRaDec(ra, offset2_dec, object);} ");
		sprintf(scanTar_data_4[16],"if (scanType == 3) {sky6StarChart.Find(object);                           ");
		sprintf(scanTar_data_4[17],"			sky6ObjectInformation.Property(58);                   ");
		sprintf(scanTar_data_4[18],"			az = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[19],"			sky6RASCOMTele.SlewToAzAlt(offset2_az, el, object); } ");
		sprintf(scanTar_data_4[20],"if (scanType == 4) {sky6StarChart.Find(object);			      ");
		sprintf(scanTar_data_4[21],"			sky6ObjectInformation.Property(59);		      ");
		sprintf(scanTar_data_4[22],"			el = sky6ObjectInformation.ObjInfoPropOut;	      ");
		sprintf(scanTar_data_4[23],"			sky6RASCOMTele.SlewToAzAlt(az, offset2_el, object); } ");
		sprintf(scanTar_data_4[34],"while(SlewComplete != 1){ ");
		sprintf(scanTar_data_4[25],"         SlewComplete = sky6Web.IsSlewComplete;} ");
		sprintf(scanTar_data_4[26],"Out = 'StepSlewDone.'; ");
		sprintf(scanTar_data_4[27],"/* Socket End Packet */ ");



				//SENDING DATA INSTRUCTIONS TO SERVER 
				/************************************/
			      	//-----Move to offset position - Begin of SCAN ------
	
				if (debug_flag) printf("\n");
				for(i = 0; i<=scanTar_data_4_nlines ; i++)
				      	{
					if( send(sock , scanTar_data_4[i] , strlen(scanTar_data_4[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",scanTar_data_4[i]);
				       	}
				if (debug_flag) printf("\n");
			
				//Receiving data from server - inicial procedures
				usleep(TX_DELAY);
			       	log_info("Scan Target data instructions sent to remote server.");
				if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");
				
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

				//=================================================================
//				close(sock);
				//=================================================================
//				log_info("Socket TCP closed.");
//				if (debug_flag) printf("Socket TCP closed.\n\n");
	
				// VERIFYING IF SO FAR SO GOOD - for Offset Position
				if ( memcmp(server_reply,"StepSlewDone.|No error. Error = 0.", strlen(server_reply)) == 0 )
				{
				
					if (verbose_flag) printf("StepSlewDone. Ready to next step slew.\n\n");
	
				}else
				{
					log_info("Server reply not understandable.");
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
					if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			if (debug_flag) printf("Socket TCP closed.\n");
				
		}
		offset1_ra = ra + offset;
		offset2_ra = ra - offset;
		offset1_dec = dec + offset;
		offset2_dec = dec - offset;
		offset1_az = az + offset;
		offset2_az = az - offset;
		offset1_el = el + offset;
		offset2_el = el - offset;
	
		subscan_var = 1;

	}


	//======================================
	// End of SCAN - SLEW Back to the Object
	//======================================

	//Rotina Javascript para coleta das coordenadas e apontamento para o objeto
	//-------------------------------------------------------------------------
	char scanTar_data_5[19][150]          ;	// Data to send to server
	int scanTar_data_5_nlines = 19        ;	// Getter commands number of lines from scanTar_data[].

	sprintf(scanTar_data_5[0],"/* Java Script */ ");
	sprintf(scanTar_data_5[1],"/* Socket Start Packet */ ");
	sprintf(scanTar_data_5[2],"var Out,SlewComplete; ");
	sprintf(scanTar_data_5[3],"var object = '%s';    ", object);
	sprintf(scanTar_data_5[4],"sky6StarChart.Find(object); ");
	sprintf(scanTar_data_5[5],"sky6ObjectInformation.Property(54); ");
	sprintf(scanTar_data_5[6],"  var ra = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_5[7],"sky6ObjectInformation.Property(55); ");
	sprintf(scanTar_data_5[8],"  var dec = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_5[9],"sky6ObjectInformation.Property(77); ");
	sprintf(scanTar_data_5[10],"  var ra_rate = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_5[11],"sky6ObjectInformation.Property(78); ");
	sprintf(scanTar_data_5[12],"  var dec_rate = sky6ObjectInformation.ObjInfoPropOut; ");
	sprintf(scanTar_data_5[13],"sky6RASCOMTele.SlewToRaDec(ra, dec, object); ");
	sprintf(scanTar_data_5[14],"while(slewComplete != 1){ ");
	sprintf(scanTar_data_5[15],"     slewComplete = sky6Web.IsSlewComplete; ");
	sprintf(scanTar_data_5[16],"} ");
	sprintf(scanTar_data_5[17],"sky6RASCOMTele.SetTracking(1, 0, ra_rate, dec_rate); ");
	sprintf(scanTar_data_5[18],"Out='ScanTar.Done.'; ");
	sprintf(scanTar_data_5[19],"/* Socket End Packet */ ");



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
	if (debug_flag) printf("\nConnection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


	// Routine for Op_Mode 
	// - - - - - - - - - - - - - - -
	// THIS IS THE OP_MODE ROUTINE - BEGIN
	// Use semaphore as a mutex (lock) by waiting for writer to increment it
	if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
	{
		opmode_var->opmode = SLEW;
		sem_post(semptr_opmode);
	}
	// THIS IS THE OP_MODE ROUTINE = END
	// ---------------------------------
	if (debug_flag)
	{
		for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
		{
			if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
		}
		printf("\n----------------------\n");
		printf("Operation Mode Set:   \n");
		printf("----------------------\n");
		printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
		printf("----------------------\n\n");
	}
	
	//SENDING DATA INSTRUCTIONS TO SERVER 
	/************************************/
      	//-----To get the coordenate to perform the scan------
	
	if (debug_flag) printf("\n");
	for(i = 0; i<=scanTar_data_5_nlines ; i++)
      	{
		if( send(sock , scanTar_data_5[i] , strlen(scanTar_data_5[i]) , 0) < 0)
		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
		if (debug_flag) printf("%s\n",scanTar_data_5[i]);
       	}
	if (debug_flag) printf("\n");

	//Receiving data from server - inicial procedures
	usleep(TX_DELAY);
       	log_info("Scan Target data instructions sent to remote server.");
	if (debug_flag) printf("Scan Target data instructions sent to remote server.\n");

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
	if ( memcmp(server_reply,"ScanTar.Done.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		if (verbose_flag) printf("Scan process finished! Pointing...\n\n");
		// Routine for Op_Mode = SKYDIP
		// - - - - - - - - - - - - - - -
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
		{
			//opmode_var->object = SKY;
			opmode_var->opmode = TRACK;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag)
		{
			for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)       //OPMODEEND = 200
			{
				if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
			}
			printf("----------------------\n");
			printf("Operation Mode Set:   \n");
			printf("----------------------\n");
			printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
			printf("----------------------\n\n");
		}

		log_info("Scan Target Done. Telescope is back to %s. Tracking...", object);
		if (debug_flag)  printf("\rScan Target Done! Telescope is back to %s. Tracking...\n\n", object);
		if (verbose_flag)  printf("\rTelescope is back to %s. Tracking...\n\n", object);

	}else
	{
		log_info("Server reply not understandable.");
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
		if (debug_flag || verbose_flag) printf("\rScanTar Failure. Probably Server Error. Telescope is Tracking off.\n\n");
	
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
	if (debug_flag) printf("End of scanTar.\n\n");

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
