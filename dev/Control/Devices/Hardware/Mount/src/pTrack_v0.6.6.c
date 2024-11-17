/*
        =================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        =================================================================================

        Point and Track v.0.6.6
*/
#define VERSION "0.6.6"
/*
  	---------------------------------------------------------------------------------
        Este programa visa realizar o apontamento para o objeto (slew) e posteriormente
	realizar o acompanhamento (track) utilizando as definicoes de track (velocidade)
	de acordo com o tracking rate fornecido pelo TheSkyX do objeto.

        TheSkyX used classes:
	---------------------------------------------------------------------------------
        sky6RASCOMTele
	sky6ObjectInformation
	---------------------------------------------------------------------------------

        Autor: Tiago Giorgetti
        Email: tiago.giorgetti@craam.mackenzie.br

	---------------------------------------------------------------------------------

        Histórico:
        _________________________________________________________________________________
         Versão |  Data         |       Atualização
        ---------------------------------------------------------------------------------
          0.1   |  05-03-2020   | Primeira versão, realiza a função e apresenta na tela
                |               | informação do servidor com a conclusão da instrução.
        ---------------------------------------------------------------------------------
	  0.2	|  05-03-2020   | Implementação de argumentos na execução com opcoes
                |               | de objetos para o apontamento e posterior tracking.
	---------------------------------------------------------------------------------
	  0.3	|  10-03-2020	| Implementação de arquivo de configuração com uso da
		|		| lib "../inc/confuse.h", localizado em ../etc/HAX.config
	---------------------------------------------------------------------------------
	  0.4	|  10-03-2020   | Implementação de um sistema de log com mensagens de
		|               | erros ou informações sobre o sistema.
	---------------------------------------------------------------------------------
	  0.5	|  11-03-2020	| Atualização da libConfuse para versão 3.2 para permitir
		|		| suporte a CFGF_IGNORE_UNKNOWN por default.
	---------------------------------------------------------------------------------
          0.6   |  23-06-2021   | Implementação de shared memory para carregamento das 
           	| 		| variáveis provenientes do arquivo de configuracao.
		|		| Modificacao no uso de argumentos para passagem de 
		|		| parametros.
		|		| Modificacao no envio de javascript para servidor, com a
		|		| separação entre verificar dados do objeto e instruções 
		|		| de envio dos codigos para apontamento e acompanhamento.
	  	|		|
	  0.6.1	|  17-08-2021	| Melhoria no tratamento das respostas do servidor com a
		|		| utilização da função memcmp() para comparar strings.
	        |		| Atualização do pTrack.h, edição do usage. Varios bugs.
		|		|
	  0.6.2 |  18-08-2021	| Correcao de varios bugs de falha no recebimento de   
		|		| mensagens de resposta do servidor, tratamento de novas
		|		| mensagens de erro, incluindo tratamento do "find home",
		|		| com o 'resume' do slew para o objeto.
		|		|
	  0.6.3 |  29-09-2021	| Implementacao de GETOPS (primeira iniciativa): -h, -p,
		|		| -u, -c, -d, -f, --opmode, PARK, UNPARK, HOME, CONNECT,
		|		| DISCONNECT. Correcao de alguns bugs.
	  	|  		|
	  0.6.4 |  01-10-2021	| Revisao da aplicacao dos modos de operacao (TRACK, 
	  	|		| SLEW, PARKED, SKY, HOME, STALL). Revisao da aplicacao
		|		| dos modos de operacao e gravacao na shared memory.
		|		|
		|		|
	  0.6.5 |  04-10-2021	| Implementacao --version, --debug e --v ou --verbose.
		| 		| Implementacao --ra, --dec, --az e --el. Correcao de 
		|		| erros com aplicacao de opmode para STALL quado slew
		|		| for abortado por estar Limit Exceeded.
		|		|
	  0.6.6 |  22-03-2022   | Inclusao de prints dos scripts para debug_flag. 
	
		|		|
	  0.6.8	|		| Implementacao do --joy. Revisao para tratar bugs.
	
	---------------------------------------------------------------------------------
	  0.7	|		| Implementação de funções para: PARK, UNPARK, HOME, 
	  			  DISCONNECT, como parametros opcionais ou objeto.
				  Verificação o funcionamento do Op_MODE.
	_________________________________________________________________________________

	Web References:

	Sistema de logging
	..................
	http://github.com/rxi/log.c

	Manipulando arquivos
	....................
	http://www.tutorialspoint.com/c_standard_library/c_function_fopen.htm

	Criando arquivos rotativos por ano
	..................................
	http://www.vivaolinux.com.br/topico/C-C++/Retornar-o-ano-atual-do-sistema-em-uma-variavel

	Uso de arquivo de configuracao com a biblioteca "confuse.h"
	...........................................................
	http://www.nongnu.org/confuse/manual/index.html
	http://savannah.nongnu.org/download/confuse/
	http://www.vivaolinux.com.br/artigo/Criando-programas-com-suporte-a-arquivos-de-configuracao-com-a-libConfuse?pagina=5

	Lib "confuse.h" inicialmente instalada via apt-get install libconfuse-dev (v.3.0) (/usr/include/confuse.h)
	Biblioteca copiada para ../inc/confuse.h de modo a reunir todas as libs extras em somente um local.

	https://abi-laboratory.pro/index.php?view=changelog&l=libconfuse&v=3.2

	[v3.0]
	* Support for handling unknown options.  The idea is to provide future
	  proofing of configuration files, i.e. if a new parameter is added, the
	  new config file will not fail if loaded in an older version of your
	  program.  See the `CFGF_IGNORE_UNKNOWN` flag in the documenation for
	  more information.  Idea and implementation by Frank Hunleth.

	[v3.1]
	* Refactored `CFGF_IGNORE_UNKNOWN` support, libConfuse now properly
	  ignores any type and sub-section without the need for declaring an
	  `__unknown` option.  When the flag is set all unknown options,
	  including unknown sub-sections with, in turn, unknown options, are
	  now fully ignored

	Caso o arquivo de configuração possua opções sem uso pelo programa, ocorria Falha de Segmentação durante a execução.
	Para contornar este problema, a libconfuse foi atualizada para a versão [v3.2] conforme procedimento abaixo:


	**** Procedimento para atualização da libConfuse: ****

        mv /usr/include/confuse.h /usr/include/confuse.h.old
	mv /opt/HAX/Control/Devices/Hardware/Mount/inc/confuse.h /opt/HAX/Control/Devices/Hardware/Mount/inc/confuse.h.old

	libconfuse-common_3.2.2+dfsg-1_all.deb
	wget http://snapshot.debian.org/archive/debian/20180819T225235Z/pool/main/c/confuse/libconfuse-common_3.2.2%2Bdfsg-1_all.deb

	libconfuse2_3.2.2+dfsg-1_amd64.deb
	wget http://snapshot.debian.org/archive/debian/20180819T225235Z/pool/main/c/confuse/libconfuse2_3.2.2%2Bdfsg-1_amd64.deb

	libconfuse-dev_3.2.2+dfsg-1_amd64.deb
	wget http://snapshot.debian.org/archive/debian/20180819T225235Z/pool/main/c/confuse/libconfuse-dev_3.2.2%2Bdfsg-1_amd64.deb

	dpkg -i libconfuse-common_3.2.2+dfsg-1_all.deb
	dpkg -i libconfuse2_3.2.2+dfsg-1_amd64.deb
	dpkg -i libconfuse-dev_3.2.2+dfsg-1_amd64.deb

	cp /usr/include/confuse.h /opt/HAX/Control/Devices/Hardware/Mount/inc/


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
#include "pTrack.h"	//Definitions for pTrack
#include "opmode.h"	//Definitions for Operation Modes of the Telescope
#include "cfg_buffer.h" //Atualizacao para versao 0.6 - cfgCatcher




#define AccessPermsIN 0644



        /***
        *     P R E L I M I N A R Y     F U N C T I O N S
        *****************************************************************/

void report_and_exit(const char *);
void copy_char ( char a[], char b[], char x);
int count_char ( char a[], char x);
int print_usage();     			// from pTrack.h


	// Tratamento de parametros via getopt()
	// -------------------------------------

//Opt Flag Variables
static int version_flag         ;
static int debug_flag           ;
static int opmode_flag          ;
static int joy_flag             ;
static int stall_flag		;

int ra_flag             = 0     ;
int dec_flag            = 0     ;
int az_flag             = 0     ;
int el_flag             = 0     ;

int help_flag           = 0     ;
int park_flag           = 0     ;
int unpark_flag         = 0     ;
int connect_flag        = 0     ;
int disconnect_flag     = 0     ;
int verbose_flag        = 0     ;
int findhome_flag	= 0	;
int sky_flag		= 0	;

//Opt Arguments
char *ra_value          = NULL  ;
char *dec_value         = NULL  ;
char *az_value          = NULL  ;
char *el_value          = NULL  ;
char *object            = NULL  ;

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
			{"opmode",  no_argument,        &opmode_flag,   1},
			{"joy",     no_argument,        &joy_flag,      1},
			{"stall",   no_argument,	&stall_flag,	1},
			/* These options dont't set a flag. We distinguish them by their indices. */
			{"ra",  required_argument,      0,      0},
			{"dec", required_argument,      0,      0},
			{"az",  required_argument,      0,      0},
			{"el",  required_argument,      0,      0},
			{"help",        no_argument,    0,      'h'},
			{"park",        no_argument,    0,      'p'},
			{"unpark",      no_argument,    0,      'u'},
			{"connect",     no_argument,    0,      'c'},
			{"disconnect",  no_argument,    0,      'd'},
			{"verbose",     no_argument,    0,      'v'},
			{"findhome",	no_argument,	0,	'f'},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "hpucdvf", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
	
		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				if (memcmp( long_options[option_index].name,"ra",2)==0)
				{
					ra_flag = 1;
					optflag_ctr++;
					//printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					ra_value = optarg;
						//printf("%s\n",ra_value); }
				}
				if (memcmp( long_options[option_index].name,"dec",3)==0)
				{
					dec_flag = 1;
					optflag_ctr++;
					//printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					dec_value = optarg;
						//printf("%s\n",dec_value); }
				}
				if (memcmp( long_options[option_index].name,"az",2)==0)
				{
					az_flag = 1;
					optflag_ctr++;
					//printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					az_value = optarg;
						//printf("%s\n",az_value); }
				}
				if (memcmp( long_options[option_index].name,"el",2)==0)
				{
					el_flag = 1;
					optflag_ctr++;
					//printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					el_value = optarg;
						//printf("%s\n",el_value); }
				}
				break;
			case 'h':
				help_flag = 1;
				optflag_ctr++;
				//printf("help_flag: %d\n",help_flag);
				break;
			case 'p':
				park_flag = 1;
				optflag_ctr++;
				//printf("park_flag: %d\n",park_flag);
				break;
			case 'u':
				unpark_flag = 1;
				optflag_ctr++;
				//printf("unpark_flag: %d\n",unpark_flag);
				break;
			case 'c':
				connect_flag = 1;
				optflag_ctr++;
				//printf("connect_flag: %d\n",connect_flag);
				break;
			case 'd':
				disconnect_flag = 1;
				optflag_ctr++;
				//printf("disconnect_flag: %d\n",disconnect_flag);
				break;
			case 'v':
				verbose_flag = 1;
				optflag_ctr++;
				//printf("verbose_flag: %d\n",verbose_flag);
				break;
			case 'f':
				findhome_flag = 1;
				optflag_ctr++;
				//printf("findhome_flag: %d\n",findhome_flag);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				exit(1);
				break;
			default:
				abort();
		}
	}

	/* Instead of reporting ‘--verbose’
	 * and ‘--brief’ as they are encountered,
	 * we report the final status resulting from them.
	 */

	//if (version_flag)
		//printf("version_flag: %d\n", version_flag);
	//if (debug_flag)
		//printf("debug_flag:   %d\n", debug_flag);
	//if (opmode_flag)
		//printf("opmode_flag:  %d\n", opmode_flag);
	//if (joy_flag)
		//printf("joy_flag:     %d\n", joy_flag);
	
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
		if (diff > 1)
		{
			printf("\n");
			printf("pTrack: Too many arguments for object. Try -h or --help.\n");
			exit(1);
		}
		if (debug_flag) printf("\rObject = %s                 \n",object);
	}
	
	// Getting flag from Special Object Arguments
	// -------------------------------------------
	if (object)
	{
		if (memcmp(object,"PARK",4)        == 0)	park_flag = 1;
		if (memcmp(object,"UNPARK",6)      == 0)	unpark_flag = 1;
		if (memcmp(object,"CONNECT",7)     == 0)	connect_flag = 1;
		if (memcmp(object,"DISCONNECT",10) == 0)	disconnect_flag = 1;
		if (memcmp(object,"HOME",4)        == 0)	findhome_flag = 1;
		if (memcmp(object,"STALL",5)       == 0)	stall_flag = 1;
		if (memcmp(object,"sky",3)         == 0)	sky_flag = 1;
	}
	//--------------------------------------------


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
		if(opmode_flag)		printf("opmode_flag = %d\n", opmode_flag);
		if(joy_flag) 		printf("joy_flag = %d\n", joy_flag);
		if(stall_flag) 		printf("stall_flag = %d\n", stall_flag);
	
		if(ra_flag) 		printf("ra_flag = %d\n", ra_flag);
		if(dec_flag) 		printf("dec_flag = %d\n", dec_flag);
		if(az_flag) 		printf("az_flag = %d\n", az_flag);
		if(el_flag) 		printf("el_flag = %d\n", el_flag);

		if(help_flag) 		printf("help_flag = %d\n", help_flag);
		if(park_flag) 		printf("park_flag = %d\n", park_flag);
		if(unpark_flag) 	printf("unpark_flag = %d\n", unpark_flag);
		if(connect_flag) 	printf("connect_flag = %d\n", connect_flag);
		if(disconnect_flag) 	printf("disconnect_flag = %d\n", disconnect_flag);
		if(verbose_flag) 	printf("verbose_flag = %d\n", verbose_flag);
		if(findhome_flag) 	printf("findhome_flag = %d\n", findhome_flag);
		if(sky_flag) 		printf("sky_flag = %d\n", sky_flag);
	}

	
	if (argc < 2)
	{
		puts("pTrack: Too few arguments or options. Try -h or --help.");
		exit(1);
	}

//	if (optind > 1 && !object)
//	{
//		puts("pTrack: Too few or incomplete arguments. Try -h or --help.");
//		exit(1);
//	}

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

	if (joy_flag == 1 && argc > 2)
	{
		puts("pTrack: The Joystick fuction must be used without another option! Try -h or --help.");
		exit(1);
	}

	if (stall_flag == 1 && argc > 2)
	{
		puts("pTrack: The Stall fuction must be used without another option! Try -h or --help.");
		exit(1);
	}


	if (opmode_flag == 1 && argc > 2)
	{
		puts("pTrack: For opmode information must be used without another option! Try -h or --help.");
		exit(1);
	}

	if (version_flag == 1 && argc > 2)
	{
		puts("pTrack: For version information must be used without another option! Try -h or --help.");
		exit(1);
	}

	if ((connect_flag == 1 && object && memcmp(object,"CONNECT",7) != 0) && (connect_flag == 1 && verbose_flag != 1 && debug_flag != 1 && unpark_flag != 1))
	{
		puts("pTrack: Options denied! Connect option only can be combined with -v , --debug or -u (Unpark) options! Try -h or --help.");
		exit(1);
	}

	if (park_flag == 1 && unpark_flag == 1)
	{
		puts("pTrack: Options denied! You must decide before if you want park or unpark the telescope. Try -h or --help.");
		exit(1);
	}

	if (park_flag == 1 && object && memcmp(object,"PARK",4) != 0 && memcmp(object,"CONNECT",7) != 0)
	{
		puts("pTrack: Options denied! You need to Park or observe something? Try -h or --help.");
		exit(1);
	}

	if ((disconnect_flag == 1 && object && memcmp(object,"DISCONNECT",4) != 0 && memcmp(object,"PARK",4) != 0))
	{
		puts("pTrack: Options denied! Disconnect option only can be combined with --debug, -v or -p options or PARK object. Try -h or --help.");
		exit(1);
	}

	if ((ra_flag && az_flag) || (ra_flag && el_flag) || (dec_flag && az_flag) || (dec_flag && el_flag) || (ra_flag && !dec_flag) || (ra_flag && !dec_flag) || (!ra_flag && dec_flag))
	{
		printf("	pTrack: Wrong coordinates! Use the correct system coordinates.\n");
		printf("		For equatorial system: --ra (Right Ascention) and --dec (Declination)\n");
		printf("		For horizon system: --az (Azimute) and --el (Elevation)\n");
		printf("    ------------------------------------------------------------------------------------\n");
		printf("		Or type the option -h or --help\n");
		exit(1);
	}




	if (debug_flag) printf("\nRun the pTrack instructions!\n\n");

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
	//proposta novo nome: Ephem_ano.log

        FILE *fp;

        if ((fp=fopen(Nome_Arquivo_Log, "a"))==NULL)
        {
                printf("Can´t open/create the log file! Check directory permissions.\n\n");
                exit(1);
        }

        log_set_fp(fp);
	log_set_quiet(1);
	//-----------------------------------------------------------------------------------





	// DECLARACAO VARIAVEIS DA ROTINA DE COMUNICACAO COM SERVIDOR
	// ----------------------------------------------------------

        int i,j                                 ;       //Generic counter
        int sock                                ;       //Socket variable
        struct sockaddr_in server               ;       //Socket variable
        char server_reply[RCV_BUFFER_SIZE]      ;       //Socket variable

	

	// Javascript instructions for TheSkyX server ( Object Verification Instruction )
	// ------------------------------------------
	char find_object_data[7][50]            ;       // Check if object exists in the server [Line][Colum].
	int find_object_data_nlines = 7		;       // Number of lines from chk_object_data[].

	sprintf(find_object_data[0],"/* Java Script */		");
	sprintf(find_object_data[1],"/* Socket Start Packet */	");
	sprintf(find_object_data[2],"var Out;			");
	sprintf(find_object_data[3],"var obj='%s';		",object);
	sprintf(find_object_data[4],"sky6StarChart.Find(obj);	");
	sprintf(find_object_data[5],"Out = 'Object found.';	");
	sprintf(find_object_data[6],"/* Socket End Packet */	");




	// Javascript instructions for TheSkyX server (Point target and Track)
	// ------------------------------------------
	char set_pTrack_data[28][100]           ;	// Data to send to server
	int set_pTrack_data_nlines = 28         ;	// Getter commands number of lines from set_pTrack_data[].
							// If you add a command remember to modify, add, etc a new
							// set_pTrack_data_nlines
	
	sprintf(set_pTrack_data[0],"/* Java Script */								");
	sprintf(set_pTrack_data[1],"/* Socket Start Packet */							");
	sprintf(set_pTrack_data[2],"var Out;									");
	sprintf(set_pTrack_data[3],"var err;									");
	sprintf(set_pTrack_data[4],"var obj='%s';								",object);
	sprintf(set_pTrack_data[5],"sky6StarChart.LASTCOMERROR = 0;						");
	sprintf(set_pTrack_data[6],"sky6StarChart.Find(obj);							");
	sprintf(set_pTrack_data[7],"err = sky6StarChart.LASTCOMERROR;						");
	sprintf(set_pTrack_data[8],"if (err != 0){								");
	sprintf(set_pTrack_data[9],"        Out = Target + ' not found.';					");
	sprintf(set_pTrack_data[10],"}else{									");
	sprintf(set_pTrack_data[11],"        sky6ObjectInformation.Property(54);				");
	sprintf(set_pTrack_data[12],"        var targetRA = sky6ObjectInformation.ObjInfoPropOut;		");
	sprintf(set_pTrack_data[13],"        sky6ObjectInformation.Property(55);				");
	sprintf(set_pTrack_data[14],"        var targetDec = sky6ObjectInformation.ObjInfoPropOut;		");
	sprintf(set_pTrack_data[15],"        sky6ObjectInformation.Property(77);				");
	sprintf(set_pTrack_data[16],"        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;	");
	sprintf(set_pTrack_data[17],"        sky6ObjectInformation.Property(78);				");
	sprintf(set_pTrack_data[18],"        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;	");
	sprintf(set_pTrack_data[19],"        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj);		");
	sprintf(set_pTrack_data[20],"        var slewComplete = sky6Web.IsSlewComplete;				");
	sprintf(set_pTrack_data[21],"        while(slewComplete == 0){						");
	sprintf(set_pTrack_data[22],"                slewComplete = sky6Web.IsSlewComplete;			");
	sprintf(set_pTrack_data[23],"        }									");
	sprintf(set_pTrack_data[24],"        sky6RASCOMTele.SetTracking(1,0,tracking_rateRA,tracking_rateDec);	");
	sprintf(set_pTrack_data[25],"}										");
	sprintf(set_pTrack_data[26],"Out = 'isSlew=' + slewComplete + '.';					");
	sprintf(set_pTrack_data[27],"/* Socket End Packet */							");



	// Javascript instructions for TheSkyX server (Find Home)
	// ------------------------------------------
	char find_Home_data[12][50]              ;       //Instructions to find home
	int find_Home_data_nlines = 12           ;       //Getter commands number of lines

	sprintf(find_Home_data[0],"/* Java Script */			");
	sprintf(find_Home_data[1],"/* Socket Start Packet */		"); 
	sprintf(find_Home_data[2],"var Out;				");                 
	sprintf(find_Home_data[3],"if (!sky6RASCOMTele.IsConnected){	");
	sprintf(find_Home_data[4],"	sky6RASCOMTele.Connect();	");
	sprintf(find_Home_data[5],"	sky6RASCOMTele.FindHome();	");   
	sprintf(find_Home_data[6],"	Out='Homed.';			");
	sprintf(find_Home_data[7],"}else{				");
	sprintf(find_Home_data[8],"	sky6RASCOMTele.FindHome();	");   
	sprintf(find_Home_data[9],"	Out='Homed.';}			");
	sprintf(find_Home_data[10],"Out;				");
	sprintf(find_Home_data[11],"/* Socket End Packet */		");




	// Javascript instructions for TheSkyX server (Park AND Disconnect)
	// ------------------------------------------
	char parkD_data[13][50]                  ;       //Instructions to Park and Disconnect
	int parkD_data_nlines = 13               ;       //Getter commands number of lines

	sprintf(parkD_data[0],"/* Java Script */			");
	sprintf(parkD_data[1],"/* Socket Start Packet */		");
	sprintf(parkD_data[2],"var Out;					");
	sprintf(parkD_data[3],"if (!sky6RASCOMTele.IsConnected){	");
	sprintf(parkD_data[4],"  Out = 'AlreadyDisconnected.';		");
	sprintf(parkD_data[5],"}else if (!sky6RASCOMTele.IsParked()){	");
	sprintf(parkD_data[6],"  sky6RASCOMTele.Park();			");
	sprintf(parkD_data[7],"  Out = 'Parked.Disconnected.';		");
	sprintf(parkD_data[8],"}else if (sky6RASCOMTele.IsParked()){	");
	sprintf(parkD_data[9],"  sky6RASCOMTheSky.DisconnectTelescope();");
	sprintf(parkD_data[10],"  Out = 'AlreadyParked.Disconnected.';}	");
	sprintf(parkD_data[11],"Out;					");
	sprintf(parkD_data[12],"/* Socket End Packet */			");


	

	// Javascript instructions for TheSkyX server (Park and Do NOT Disconnect)
	// ------------------------------------------
	char park_data[11][50]                 ;       //Instructions to Park and NOT Disconnect
	int park_data_nlines = 11              ;       //Getter commands number of lines

	sprintf(park_data[0],"/* Java Script */					");
	sprintf(park_data[1],"/* Socket Start Packet */				");
	sprintf(park_data[2],"var Out;						");
	sprintf(park_data[3],"if(!sky6RASCOMTele.IsConnected){			");
	sprintf(park_data[4],"        Out='AlreadyIsDisconnected.';		");
	sprintf(park_data[5],"}else if (!sky6RASCOMTele.IsParked()){		");
	sprintf(park_data[6],"         sky6RASCOMTele.ParkAndDoNotDisconnect();	");
	sprintf(park_data[7],"         Out = 'Parked.';				");
	sprintf(park_data[8],"}else{Out='AlreadyParked.';}			");
	sprintf(park_data[9],"Out;						");
	sprintf(park_data[10],"/* Socket End Packet */				");




	// Javascript instructions for TheSkyX server (UnPark)
	// ------------------------------------------
	char unpark_data[9][50]			;	//Instructions to UnPark
	int unpark_data_nlines = 9		; 	//Getter commands number of lines

	sprintf(unpark_data[0],"/* Java Script */			");
	sprintf(unpark_data[1],"/* Socket Start Packet */		");
	sprintf(unpark_data[2],"var Out;				");
	sprintf(unpark_data[3],"if (sky6RASCOMTele.IsParked()){		");
	sprintf(unpark_data[4],"	 sky6RASCOMTele.Unpark();	");
	sprintf(unpark_data[5],"	 Out = 'NotParked.';		");
	sprintf(unpark_data[6],"}else{Out='AlreadyNotParked.';}		");
	sprintf(unpark_data[7],"Out;					");
	sprintf(unpark_data[8],"/* Socket End Packet */			");




	// Javascript instructions for TheSkyX server (Connect and Park)
	// ------------------------------------------
	char connectP_data[15][50]              ;       //Instructions to Connect and Park
	int connectP_data_nlines = 15           ;       //Getter commands number of lines

	sprintf(connectP_data[0],"/* Java Script */                                ");
	sprintf(connectP_data[1],"/* Socket Start Packet */                        ");
	sprintf(connectP_data[2],"var Out;                                         ");
	sprintf(connectP_data[3],"if (!sky6RASCOMTele.IsConnected ){		   ");
	sprintf(connectP_data[4],"       sky6RASCOMTele.Connect();                 ");
	sprintf(connectP_data[5],"       sky6RASCOMTele.ParkAndDoNotDisconnect();  ");
	sprintf(connectP_data[6],"       Out = 'Connected.Parked.';		   ");
	sprintf(connectP_data[7],"}else if(sky6RASCOMTele.IsConnected &&	   ");
	sprintf(connectP_data[8],"!sky6RASCOMTele.IsParked()){			   ");
	sprintf(connectP_data[9],"	sky6RASCOMTele.ParkAndDoNotDisconnect();   ");
	sprintf(connectP_data[10],"	Out='AlreadyConnected.Parked.';		   ");
	sprintf(connectP_data[11],"}else if (sky6RASCOMTele.IsParked()){	   ");
	sprintf(connectP_data[12],"	Out='AlreadyConnected.AlreadyParked.';}	   ");
	sprintf(connectP_data[13],"Out; 					   ");
	sprintf(connectP_data[14],"/* Socket End Packet */			   ");



	// Javascript instructions for TheSkyX server (Connect or (Connect and UnPark))
	// ------------------------------------------
	char connect_data[9][50]		;	//Instructions to Connect
	int connect_data_nlines = 9		; 	//Getter commands number of lines

	sprintf(connect_data[0],"/* Java Script */			");
	sprintf(connect_data[1],"/* Socket Start Packet */		");
	sprintf(connect_data[2],"var Out;				");
	sprintf(connect_data[3],"if (!sky6RASCOMTele.IsConnected){	");
	sprintf(connect_data[4],"	 sky6RASCOMTele.Connect();	");
	sprintf(connect_data[5],"	 Out = 'Connected.';		");
	sprintf(connect_data[6],"}else{Out='AlreadyConnected.';}	");
	sprintf(connect_data[7],"Out;					");
	sprintf(connect_data[8],"/* Socket End Packet */		");



	// Javascript instructions for TheSkyX server (Disconnect)
	// ------------------------------------------
	char disconnect_data[10][50]             ;       //Instructions to Disconnect
	int disconnect_data_nlines = 10          ;       //Getter commands number of lines

	sprintf(disconnect_data[0],"/* Java Script */					");
	sprintf(disconnect_data[1],"/* Socket Start Packet */				");
	sprintf(disconnect_data[2],"var Out;						");
	sprintf(disconnect_data[3],"if (sky6RASCOMTele.IsConnected){			");
	sprintf(disconnect_data[4],"     sky6RASCOMTheSky.DisconnectTelescope();	");
	sprintf(disconnect_data[5],"     Out = 'Disconnected.';				");
	sprintf(disconnect_data[6],"}else if(!sky6RASCOMTele.IsConnected){		");
	sprintf(disconnect_data[7],"	 Out='AlreadyDisconnected.';}			");
	sprintf(disconnect_data[8],"Out;						");
	sprintf(disconnect_data[9],"/* Socket End Packet */				");


	// Javascript instructions for TheSkyX server (STALL)
	// ------------------------------------------
	char stall_data[13][50]          ;       //Instructions to Abort or STALL
	int stall_data_nlines = 13       ;       //Getter commands number of lines

	sprintf(stall_data[0],"/* Java Script */                        ");
	sprintf(stall_data[1],"/* Socket Start Packet */                ");
	sprintf(stall_data[2],"var Out;                                 ");
	sprintf(stall_data[3],"var err;                                 ");
	sprintf(stall_data[4],"sky6StarChart.LASTCOMERROR = 0;          ");
	sprintf(stall_data[5],"sky6RASCOMTele.SetTracking(0,1,0,0);     ");
	sprintf(stall_data[6],"err = sky6StarChart.LASTCOMERROR;        ");
	sprintf(stall_data[7],"if (err != 0){                           ");
	sprintf(stall_data[8],"  Out = 'Erro.';                         ");
	sprintf(stall_data[9],"}else{                                   ");
	sprintf(stall_data[10]," Out = 'Stall.';			");
	sprintf(stall_data[11],"}                                       ");
	sprintf(stall_data[12],"/* Socket End Packet */                 ");




	// Javascript instructions for TheSkyX server (Target Sky)
	// ------------------------------------------
	char sky_data[17][50]              ;       //Instructions to target Sky
	int sky_data_nlines = 17           ;       //Getter commands number of lines

	sprintf(sky_data[0],"/* Java Script */					");
	sprintf(sky_data[1],"/* Socket Start Packet */				");
	sprintf(sky_data[2],"var Out;						");
	sprintf(sky_data[3],"if(!sky6RASCOMTele.IsConnected){			");
	sprintf(sky_data[4],"	Out='NotConnected.';				");
	sprintf(sky_data[5],"}else if(sky6RASCOMTele.IsParked()){		");
	sprintf(sky_data[6],"	Out='Parked.';					");
	sprintf(sky_data[7],"}else{						");
	sprintf(sky_data[8],"	sky6RASCOMTele.GetAzAlt();			");
	sprintf(sky_data[9],"	var az = sky6RASCOMTele.dAz;			");
	sprintf(sky_data[10],"	var alt = sky6RASCOMTele.dAlt;			");
	sprintf(sky_data[11],"	var newAz = az + 5;				");
	sprintf(sky_data[12],"	sky6RASCOMTele.SlewToAzAlt(newAz, alt, '');	");
	sprintf(sky_data[13],"	sky6RASCOMTele.SetTracking(0, 1, 0, 0);		");
	sprintf(sky_data[14],"	Out='TargetSkyDone.Stall.';}			");
	sprintf(sky_data[15],"Out;						");
	sprintf(sky_data[16],"/* Socket End Packet */				");




	//Javascript instructions for TheSkyX server (Point target and Track EQUATORIAL MANUALY)
	// ------------------------------------------
	char set_equatorial_data[19][100]           ;       // Data to send to server
	int set_equatorial_data_nlines = 19         ;       // Getter commands number of lines from set_equatorial_data[].
	                                                    // If you add a command remember to modify, add, etc a new
							    // set_equatorial_data_nlines

	sprintf(set_equatorial_data[0],"/* Java Script */						");	
	sprintf(set_equatorial_data[1],"/* Socket Start Packet */					");	
	sprintf(set_equatorial_data[2],"var Out;							");
	sprintf(set_equatorial_data[3],"if(!sky6RASCOMTele.IsConnected){				");		
	sprintf(set_equatorial_data[4],"	Out='NotConnected.';					");	
	sprintf(set_equatorial_data[5],"}else if(sky6RASCOMTele.IsParked()){				");
	sprintf(set_equatorial_data[6],"	Out='Parked.';						");
	sprintf(set_equatorial_data[7],"}else{								");
	sprintf(set_equatorial_data[8],"        var targetRA = '%s';					", ra_value);
	sprintf(set_equatorial_data[9],"        var targetDec = '%s';					", dec_value);
	sprintf(set_equatorial_data[10],"        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, '');	");
	sprintf(set_equatorial_data[11],"        var slewComplete = sky6Web.IsSlewComplete;		");
	sprintf(set_equatorial_data[12],"        while(slewComplete != 1){				");
	sprintf(set_equatorial_data[13],"                slewComplete = sky6Web.IsSlewComplete;		");
	sprintf(set_equatorial_data[14],"        }							");
	sprintf(set_equatorial_data[15],"        sky6RASCOMTele.SetTracking(1, 1, 0, 0);		");
	sprintf(set_equatorial_data[16],"}								");
	sprintf(set_equatorial_data[17],"Out = 'EquatorialTargetOk.';					");
	sprintf(set_equatorial_data[18],"/* Socket End Packet */					");


	

	// Javascript instructions for TheSkyX server (Point target and Track HORIZONTAL MANUALY)
	// ------------------------------------------
	char set_horizontal_data[19][100]           ;       // Data to send to server
	int set_horizontal_data_nlines = 19         ;       // Getter commands number of lines from set_horizontal_data[].
	                                                    // If you add a command remember to modify, add, etc a new
	                                                    // set_horizontal_data_nlines

	sprintf(set_horizontal_data[0],"/* Java Script */						");
	sprintf(set_horizontal_data[1],"/* Socket Start Packet */					");
	sprintf(set_horizontal_data[2],"var Out;							");
	sprintf(set_horizontal_data[3],"if(!sky6RASCOMTele.IsConnected){				");
	sprintf(set_horizontal_data[4],"	Out='NotConnected.';					");
	sprintf(set_horizontal_data[5],"}else if(sky6RASCOMTele.IsParked()){				");
	sprintf(set_horizontal_data[6],"	Out='Parked.';						");
	sprintf(set_horizontal_data[7],"}else{								");
	sprintf(set_horizontal_data[8],"        var targetAZ = '%s';					", az_value);
	sprintf(set_horizontal_data[9],"        var targetEL = '%s';					", el_value);
	sprintf(set_horizontal_data[10],"        sky6RASCOMTele.SlewToAzAlt(targetAZ, targetEL, '');	");
	sprintf(set_horizontal_data[11],"        var slewComplete = sky6Web.IsSlewComplete;		");
	sprintf(set_horizontal_data[12],"        while(slewComplete != 1){				");
	sprintf(set_horizontal_data[13],"                slewComplete = sky6Web.IsSlewComplete;		");
	sprintf(set_horizontal_data[14],"        }							");
	sprintf(set_horizontal_data[15],"        sky6RASCOMTele.SetTracking(1, 1, 0, 0);		");
	sprintf(set_horizontal_data[16],"}								");
	sprintf(set_horizontal_data[17],"Out = 'HorizontalTargetOk.';					");
	sprintf(set_horizontal_data[18],"/* Socket End Packet */					");







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
		report_and_exit("Can't get file descriptor for operation mode data.");
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
		SKY_DIP,"SKY_DIP",      //10
		STALL,"STALL",          //50
		SLEW,"SLEW",            //100
		PARKED,"PARKED",        //150
		OPMODEEND,"OpModeEnd",  //200
	};





	//========================
	// GETTING OPERATION MODE
	if(opmode_flag)
	{
		
		for (i=0 ; keyword_opmode_map[i].opmode_index != OPMODEEND ; i++)	//OPMODEEND = 200
		{
			if ( keyword_opmode_map[i].opmode_index == opmode_var->opmode ) break;
		}

		for (j=0 ; keywordmap[j].targets != EXTRA_OBJ ; j++)	//EXTRA_OBJ = 30
		{
			if ( keywordmap[j].targets == opmode_var->object ) break;
		}
	


		printf("----------------------\n");
		printf("Operation Mode Status:\n");
		printf("----------------------\n");
		printf("Object = %d (%s)\n",opmode_var->object, keywordmap[j].keyword);
		printf("OpMode = %d (%s)\n",opmode_var->opmode, keyword_opmode_map[i].opmode_str);
		printf("----------------------\n\n");
		report_and_exit("Normal exit");
	}


	//=========================
	// Print version code
	if (version_flag)
	{
		printf("-----------------------------------------\n");
		printf("Point and Tracking - pTrack version %s\n", VERSION);
		printf("-----------------------------------------\n");
		report_and_exit("Normal exit");
	}


	//==============================================================
	//
	//  BEGIN OF INICIAL VERIFICATION ROUTINES 
	//
	//  ( CONNECT, DISCONNECT, PARK, UNPARK, HOME, SKY )
	//
	//
	//==============================================================
	
	if (!object || (memcmp(object,"PARK",4) == 0) || (memcmp(object,"UNPARK",6) == 0) || (memcmp(object,"CONNECT",7) == 0) || (memcmp(object,"DISCONNECT",10) == 0) || (memcmp(object,"HOME",4) == 0) || (memcmp(object,"STALL",5) == 0) || (memcmp(object,"sky",3) == 0))
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
		if (debug_flag) printf("Connection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);



		//SENDING DATA INSTRUCTIONS TO SERVER BASED ON OPTIONS
		/************************************************************/
      		//------------------------------------------------------------
		
		if ((connect_flag == 1 && park_flag == 0) || (connect_flag == 1 && unpark_flag == 1))
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<connect_data_nlines ; i++)
      			{
				if( send(sock , connect_data[i] , strlen(connect_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",connect_data[i]);
       		 	}
			if (debug_flag) printf("\n");

       	 		usleep(TX_DELAY);
       			log_info("Connect data instructions sent to remote server.");
			if (debug_flag) printf("Connect data instructions sent to remote server.\n");
		}

		if (connect_flag == 1 && park_flag == 1)
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<connectP_data_nlines ; i++)
	      		{
				if( send(sock , connectP_data[i] , strlen(connectP_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",connectP_data[i]);
       	 		}
			if (debug_flag) printf("\n");
 
			// Routine for Op_Mode = SLEW to PARK
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = PARK;
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
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}
       		 	usleep(TX_DELAY);
       			log_info("Connect and Park data instructions sent to remote server.");
			if (debug_flag) printf("Connect and Park data instructions sent to remote server.\n");
			

			if (debug_flag || verbose_flag) printf("\nSlewing to Park position...\n\n");
		
		}

		if (disconnect_flag == 1 && park_flag == 0)
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<disconnect_data_nlines ; i++)
      			{
				if( send(sock , disconnect_data[i] , strlen(disconnect_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",disconnect_data[i]);
       		 	}
			if (debug_flag) printf("\n");

        		usleep(TX_DELAY);
   	    		log_info("Disconnect data instructions sent to remote server.");
			if (debug_flag) printf("Disconnect data instructions sent to remote server.\n");
		}

		if (park_flag == 1 && disconnect_flag == 0 && connect_flag == 0)
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<park_data_nlines ; i++)
			{
				if( send(sock , park_data[i] , strlen(park_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",park_data[i]);
       		 	}
			if (debug_flag) printf("\n");

        		// Routine for Op_Mode = SLEW to PARK
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = PARK;
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
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}
			usleep(TX_DELAY);
       			log_info("Park and Do Not Disconnect data instructions sent to remote server.");
			if (debug_flag) printf("Park and Do Not Disconnect data instructions sent to remote server.\n\n");
			
			if (debug_flag || verbose_flag) printf("\nSlewing to Park position...\n\n");
		}

		if (park_flag == 1 && disconnect_flag == 1)
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<parkD_data_nlines ; i++)
      			{
				if( send(sock , parkD_data[i] , strlen(parkD_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",parkD_data[i]);
       		 	}
			if (debug_flag) printf("\n");
        	
			// Routine for Op_Mode = SLEW to PARK
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = PARK;
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
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
       			}
			usleep(TX_DELAY);
       			log_info("Park and Disconnect data instructions sent to remote server.");
			if (debug_flag) printf("Park and Disconnect data instructions sent to remote server.\n\n");
			
			if (debug_flag || verbose_flag) printf("\nSlewing to Park position...\n\n");
		}
	
		if (unpark_flag == 1 && connect_flag == 0)
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<unpark_data_nlines ; i++)
      			{
				if( send(sock , unpark_data[i] , strlen(unpark_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",unpark_data[i]);
        		}
        		usleep(TX_DELAY);
       			log_info("UnPark data instructions sent to remote server.");
			if (debug_flag) printf("UnPark data instructions sent to remote server.\n");
		}
	
		if (findhome_flag)
		{
			if (debug_flag) printf("\n");
			for(i = 0; i<find_Home_data_nlines ; i++)
      			{
				if( send(sock , find_Home_data[i] , strlen(find_Home_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",find_Home_data[i]);
        		}
        		if (debug_flag) printf("\n");

			// Routine for Op_Mode = SLEW to HOME
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = HOME;
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
				printf("Object = %d (HOME)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}
			usleep(TX_DELAY);
       			log_info("Find Home data instructions sent to remote server.");
			if (debug_flag) printf("Find Home data instructions sent to remote server.\n\n");
			
			if (debug_flag || verbose_flag) printf("\nFinding home position...\n\n");
		
		}
	
		if (sky_flag)
		{
			if (debug_flag) printf("\n\n");
			for(i = 0; i<sky_data_nlines ; i++)
      			{
				if( send(sock , sky_data[i] , strlen(sky_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",sky_data[i]);
        		}
			if (debug_flag) printf("\n\n");
        	
			// Routine for Op_Mode = SLEW to SKY
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = SKY;
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
				printf("Object = %d (SKY)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
      			}
			usleep(TX_DELAY);
       			log_info("Target Sky data instructions sent to remote server.");
			if (debug_flag) printf("Target Sky data instructions sent to remote server.\n\n");
			
			if (debug_flag || verbose_flag) printf("\nSlewing to sky offset position...\n\n");

		}
	
		if (stall_flag)
		{
			if (debug_flag) printf("\n\n");
			for(i = 0; i<stall_data_nlines ; i++)
      			{
				if( send(sock , stall_data[i] , strlen(stall_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",sky_data[i]);
        		}
			if (debug_flag) printf("\n\n");
        	
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
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("OpMode = %d (STALL)\n",opmode_var->opmode);
				printf("----------------------\n\n");
      			}
			usleep(TX_DELAY);
       			log_info("Stall data instructions sent to remote server.");
			if (debug_flag) printf("Stall data instructions sent to remote server.\n\n");
			
		}
	
		if (ra_flag && dec_flag)
		{
			if (debug_flag) printf("\n\n");
			for(i = 0; i<set_equatorial_data_nlines ; i++)
      			{
				if( send(sock , set_equatorial_data[i] , strlen(set_equatorial_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",sky_data[i]);
        		}
			if (debug_flag) printf("\n\n");
        	
			// Routine for Op_Mode = SLEW to RADEC
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = MANUAL;
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
				printf("Object = %d (MANUAL)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
      			}
			usleep(TX_DELAY);
       			log_info("Manual RaDec target data instructions sent to remote server.");
			if (debug_flag) printf("Manual RaDec target data instructions sent to remote server.\n\n");
			
			if (debug_flag || verbose_flag) printf("\nSlewing to Ra=%s Dec=%s position...\n\n", ra_value, dec_value);

		}
	
		if (az_flag && el_flag)
		{
			if (debug_flag) printf("\n\n");
			for(i = 0; i<set_horizontal_data_nlines ; i++)
      			{
				if( send(sock , set_horizontal_data[i] , strlen(set_horizontal_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",sky_data[i]);
        		}
			if (debug_flag) printf("\n\n");
        	
			// Routine for Op_Mode = SLEW to AZEL
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = MANUAL;
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
				printf("Object = %d (MANUAL)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
      			}
			usleep(TX_DELAY);
       			log_info("Manual AzEl target data instructions sent to remote server.");
			if (debug_flag) printf("Manual AzEl target data instructions sent to remote server.\n\n");
			
			if (debug_flag || verbose_flag) printf("\nSlewing to Az=%s El=%s position...\n\n", az_value, el_value);

		}
		//------------------------------------------------------------
    	
	
		
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
		if ( memcmp(server_reply,"Connected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope Connected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Connected.\n\n");

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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyConnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Connected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Already Connected.\n\n");

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
			report_and_exit("Normal exit");
		}

		// REPLY FROM CONNECTP_DATA
		if ( memcmp(server_reply,"Connected.Parked.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = PARK;
				opmode_var->opmode = PARKED;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (PARKED)\n",opmode_var->opmode);
				printf("----------------------\n\n");
       	 		}
			log_info("Telescope Connected and Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Connected and Parked.\n\n");
			
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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyConnected.Parked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = PARK;
				opmode_var->opmode = PARKED;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (PARKED)\n",opmode_var->opmode);
				printf("----------------------\n\n");
       	 		}
			log_info("Telescope Already Connected and Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Already Connected and Parked.\n\n");
			

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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyConnected.AlreadyParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Connected and Already Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Already Connected and Already Parked.\n\n");

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
			report_and_exit("Normal exit");
		}

		// REPLY FROM DISCONNECT_DATA
		if ( memcmp(server_reply,"Disconnected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Disconnected.\n\n");
	
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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyDisconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Already Disconnected.\n\n");
	
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
			report_and_exit("Normal exit");
		}

		// REPLY FROM PARK_DATA
		if ( memcmp(server_reply,"Parked.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = PARK;
				opmode_var->opmode = PARKED;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (PARKED)\n",opmode_var->opmode);
				printf("----------------------\n\n");
       	 		}
			log_info("Telescope is Parked and Not Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Parked and Not Disconnected.\n\n");
			
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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Already Parked.\n\n");

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
			report_and_exit("Normal exit");
		
		}else if ( memcmp(server_reply,"AlreadyIsDisconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Already Disconnected.\n\n");

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
			report_and_exit("Normal exit");
		}

		// REPLY FROM PARKD_DATA
		if ( memcmp(server_reply,"Parked.Disconnected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			// Routine for Op_Mode = PARKED
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{	
				opmode_var->object = PARK;
				opmode_var->opmode = PARKED;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (PARK)\n",opmode_var->object);
				printf("OpMode = %d (PARKED)\n",opmode_var->opmode);
				printf("----------------------\n\n");
        		}
			log_info("Telescope is Parked and Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Parked and Disconnected.\n\n");
		
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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyParked.Disconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Parked and Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Already Parked and Disconnected.\n\n");
	
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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyDisconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Disconnected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope Already Disconnected.\n\n");

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
			report_and_exit("Normal exit");
		}

		// REPLY FROM UNPARK_DATA
		if ( memcmp(server_reply,"NotParked.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope was UnParked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope was UnParked.\n\n");

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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyNotParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Not Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Already Not Parked.\n\n");

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
			report_and_exit("Normal exit");
		}
		
		// REPLY FROM FIND_HOME_DATA
		if ( memcmp(server_reply,"Homed.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			// Routine for Op_Mode = HOME Position
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{	
				opmode_var->object = HOME;
				opmode_var->opmode = TRACK;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (HOME)\n",opmode_var->object);
				printf("OpMode = %d (TRACK)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}
			log_info("Telescope was Homed. Tracking at sideral rates.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope was Homed. Tracking at sideral rates.\n\n");

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
			report_and_exit("Normal exit");
	
		}
	
		// REPLY FROM SKY_DATA
		if ( memcmp(server_reply,"TargetSkyDone.Stall.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
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

			log_info("Target to Sky done. Tracking off.");
			if (debug_flag || verbose_flag) printf("\r\nTarget to Sky done. Tracking off.\n\n");

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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"NotConnected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope not connected.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope not connected.\n\n");

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
			report_and_exit("Normal exit");
	
		}
		
		// REPLY FROM STALL_DATA
		if ( memcmp(server_reply,"Stall.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Tracking off.");
			if (debug_flag || verbose_flag) printf("\nTracking off.\n\n");

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
			report_and_exit("Normal exit");
	
		}
		
		// REPLY FROM SET_EQUATORIAL_DATA
		if ( memcmp(server_reply,"EquatorialTargetOk.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			// Routine for Op_Mode = TRACK
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{	
				opmode_var->object = MANUAL;
				opmode_var->opmode = TRACK;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (MANUAL)\n",opmode_var->object);
				printf("OpMode = %d (TRACK)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}

			log_info("Target to Ra=%s Dec=%s done. Tracking on sideral rate.", ra_value, dec_value);
			if (debug_flag || verbose_flag) printf("\r\nTarget to Ra=%s Dec=%s done. Tracking on sideral rate.\n\n", ra_value, dec_value);

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
			report_and_exit("Normal exit");
	
		}
		
		// REPLY FROM SET_HORIZONTAL_DATA
		if ( memcmp(server_reply,"HorizontalTargetOk.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			// Routine for Op_Mode = TRACK
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{	
				opmode_var->object = MANUAL;
				opmode_var->opmode = TRACK;
				sem_post(semptr_opmode);
			}
			// THIS IS THE OP_MODE ROUTINE = END
			// ---------------------------------
			if (debug_flag)
			{
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("Object = %d (MANUAL)\n",opmode_var->object);
				printf("OpMode = %d (TRACK)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}

			log_info("Target to Az=%s El=%s done. Tracking on sideral rate.", az_value, el_value);
			if (debug_flag || verbose_flag) printf("\r\nTarget to Az=%s El=%s done. Tracking on sideral rate.\n\n", az_value, el_value);

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
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"TypeError: Limits exceeded. Error = 218.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			if (debug_flag || verbose_flag) printf("Slew aborted! Limits Exceeded or target is below horizon.\n\n");
			if (debug_flag) printf("STALLING...\n");
		
			//=================================================================
			close(sock);
			//=================================================================
			sleep(1);	
			
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
			
			//Sending STALL Data Instructions
			if (debug_flag) printf("\n\n");
			for(i = 0; i<stall_data_nlines ; i++)
      			{
				if( send(sock , stall_data[i] , strlen(stall_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",sky_data[i]);
        		}
			if (debug_flag) printf("\n\n");
        	
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
				printf("----------------------\n");
				printf("Operation Mode Set:   \n");
				printf("----------------------\n");
				printf("OpMode = %d (STALL)\n",opmode_var->opmode);
				printf("----------------------\n\n");
      			}
			usleep(TX_DELAY);
       			log_info("Stall data instructions sent to remote server.");
			if (debug_flag) printf("Stall data instructions sent to remote server.\n\n");
	
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

			if ( memcmp(server_reply,"Stall.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
			{	
				if (debug_flag || verbose_flag) printf("STALL! Telescope is Tracking Off.\n\n");

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
				report_and_exit("Normal exit");
			}else
			{
				if (debug_flag) printf("STALL FAILED. REPLY NOT UNDERSTANDABLE.\n\n");
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
				report_and_exit("Normal exit");
		
			}

		}else if ( memcmp(server_reply,"undefined|No error. Error = 0.|Unknown command. Error = 303.", strlen(server_reply)) == 0 )
		{
			if (debug_flag) printf("REPLY NOT UNDERSTANDABLE.\n\n");
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
			report_and_exit("Normal exit");
		}
	}




	
	// DECLARACAO VARIAVEIS PARA EFEITOS DE TELA
	// -----------------------------------------
	//int k = 0;
	//const char spin[4]={'|', '/','-','\\'};


	// DECLARACAO VARIAVEIS PARA MAPEAMENTO DOS OBJETOS
	// ------------------------------------------------
	// Transferido para linhas mais acima.

	char ch;

	
	// Finding maped object
	
	for (i=0 ; keywordmap[i].targets != EXTRA_OBJ ; i++)	//EXTRA_OBJ = 30
	{
		if ( strcmp(object, keywordmap[i].keyword) == 0 ) break;
	}
	
	// Check if object is an EXTRA OBJECT (by index "i")
	
	if (keywordmap[i].targets == EXTRA_OBJ)
	{
		printf("\nTarget '%s' is an extra HATS object.\n", object);
		printf("\nTry another object, type only pTrack for help or press 'C' to continue and verify the server object list.\n");
		system("/bin/stty raw -echo");
		ch = getchar();
		system("/bin/stty cooked echo");
		if (ch != 'c')
		{
			log_info("Target is an extra object. Aborted by user");
			report_and_exit("\nTarget is an extra object. Aborted by user.");
		}else{
			
			// Saving the Extra Object in the Shared Memory for Operation Mode (by index "i")
			if (!sem_wait(semptr_opmode))		//Wait until semaphore != 0
			{
				opmode_var->object = keywordmap[i].targets;
				sem_post(semptr_opmode);
			}
			log_info("Operation mode Extra Object set: %d (%s) ->argument:'%s'\n", opmode_var->object, keywordmap[i].keyword, object);
			if (debug_flag) printf("Operation mode Extra Object set: %d (%s) ->argument:'%s'\n", opmode_var->object, keywordmap[i].keyword, object);
		}
	}

	// Saving the regular object in the Shared Memory for Operation Mode (by index "i")

	if (!sem_wait(semptr_opmode))		//Wait until semaphore != 0
	{
		opmode_var->object = keywordmap[i].targets;
		sem_post(semptr_opmode);
	}
	if (debug_flag) printf("Operation mode object set: %d (%s)\n", opmode_var->object, keywordmap[i].keyword);



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



	
	//FirstTry If telescope is Parked but have an option to UnPark
	// Example:  ./pTrack -u sun
	//
	if (unpark_flag)
	{
		if (debug_flag) printf("\n");
		for(i = 0; i<unpark_data_nlines ; i++)
        	{
			if( send(sock , unpark_data[i] , strlen(unpark_data[i]) , 0) < 0)
			{
				log_fatal("Could not send data to remote socket server.");
				report_and_exit("Could not send data to remote socket server.");
			}
			if (debug_flag) printf("%s\n",unpark_data[i]);
        	}
		if (debug_flag) printf("\n");

        	usleep(TX_DELAY);
       		log_info("UnPark data instructions sent to remote server.");
		if (debug_flag) printf("UnPark data instructions sent to remote server.\n");
		

		//RECEIVE A REPLY FROM THE SERVER ABOUT VERIFICATION DATA
      		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        	{
			log_fatal("Could not receive data from socket server.");
        	       	report_and_exit("Could not receive data from socket server.");
		}
		//-------------------------------------------------------
		if (debug_flag) printf("UnPark data received from socket server.\n");
	
		// REPLY FROM UNPARK_DATA
		if ( memcmp(server_reply,"NotParked.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope was UnParked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope was UnParked.\n\n");

		}else if ( memcmp(server_reply,"AlreadyNotParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Not Parked.");
			if (debug_flag || verbose_flag) printf("\r\nTelescope is Already Not Parked.\n\n");

		}
	
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
	}




	//SENDING DATA TO SERVER
	/************************************************************/
      	//SEND VERIFICATION DATA
      	//------------------------------------------------------------
	
	if (debug_flag) printf("\n");
	for(i = 0; i<find_object_data_nlines ; i++)
        {
		if( send(sock , find_object_data[i] , strlen(find_object_data[i]) , 0) < 0)
		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
		if (debug_flag) printf("%s\n",find_object_data[i]);
        }
	if (debug_flag) printf("\n");

        usleep(TX_DELAY);
       	log_info("Verification data instructions sent to remote server.");
	if (debug_flag) printf("Verification data instructions sent to remote server.\n");
	//------------------------------------------------------------
	
    	
	//RECEIVE A REPLY FROM THE SERVER ABOUT VERIFICATION DATA
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
		log_fatal("Could not receive data from socket server.");
               	report_and_exit("Could not receive data from socket server.");
	}
	//-------------------------------------------------------
	if (debug_flag) printf("Verification data received from socket server.\n");


	if (debug_flag) printf("Server Reply: [%s]\n", server_reply);
	if (debug_flag) printf("Char Counter: [%ld]\n", strlen(server_reply));

	if ( memcmp(server_reply,"TypeError: Object not found. Error = 250.|No error. Error = 0.", strlen(server_reply)) == 0 )        
	{
		log_error("Target '%s' not found.", object );
		if (debug_flag || verbose_flag) printf("\r\nTarget '%s' not found. Try another object or verify for sintax error from object name.\n", object);
		
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
		report_and_exit("Normal exit");
	
	}else if ( memcmp(server_reply,"NG|Another socket command is still in progress. Error = 219", strlen(server_reply)) == 0 )
	{
		if (debug_flag || verbose_flag) printf("\r\nAnother socket command is still in progress.\n");
		log_error("Another socket command is still in progress.");
		
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
		report_and_exit("Normal exit");
				
	}else if ( memcmp(server_reply,"Object found.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{	
		if (debug_flag || verbose_flag) printf("\r\nObject found. Target to %s selected. Slewing...\n", object);
		log_info("Object found. Target to %s selected. Slewing...", object);
			
		// Routine for Op_Mode = Slewing
		// - - - - - - - - - - - - - - - 
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))		//Wait until semaphore != 0
		{
			opmode_var->opmode = SLEW;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag) printf("Operation mode set: %d (SLEW)\n\n", opmode_var->opmode);
				//****************************
      		//SEND POINT AND TRACKING DATA
		if (debug_flag) printf("\n");
      		for(i = 0; i<set_pTrack_data_nlines ; i++)
       	 	{
			if( send(sock , set_pTrack_data[i] , strlen(set_pTrack_data[i]) , 0) < 0)
       	    		{
				log_fatal("Could not send data to remote socket server.");
				report_and_exit("Could not send data to remote socket server.");
			}
			if (debug_flag) printf("%s\n",set_pTrack_data[i]);

		}
		if (debug_flag) printf("\n");

		usleep(TX_DELAY);
		log_info("Set pTrack Data instructions sent to remote server.");
       	 	if (debug_flag) printf("Set pTrack Data instructions sent to remote server.\n");

	}else
	{
		log_error("Server Reply is strange: [%s]", server_reply);
		if (debug_flag) printf("Server Reply is strange: [%s]\n", server_reply);
		report_and_exit("Exit");
	}
	//-----------------------------------------------------

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

      	//RECEIVE A REPLY FROM THE SERVER
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
		log_fatal("Could not receive data from socket server.");
               	report_and_exit("Could not receive data from socket server.");
	}
	if (debug_flag) printf("Set pTrack data received from socket server.\n");

	//REPLY DATA ANALYZER 2 - Fast Problems before slewing
	//*********************
	if (debug_flag) printf("Server Reply: [%s]\n", server_reply);
	if (debug_flag) printf("Char Counter: [%ld]\n", strlen(server_reply));
	
	if ( memcmp(server_reply,"TypeError: A Mount command is already in progress. Error = 121.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		if (debug_flag || verbose_flag) printf("\r\nAnother mount command is already in progress.\n");
		log_error("Another mount command is already in progress.");
		
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
		report_and_exit("Normal exit");
	}

        // Is the telescope found some problems to do the slew? LIMITS EXCEEDED
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if ( memcmp(server_reply,"TypeError: Limits exceeded. Error = 218.|No error. Error = 0.", strlen(server_reply)) == 0 )
        {
                log_warn("Slew to %s aborted! Limits Exceeded or object is below horizon.", object);
                if (debug_flag || verbose_flag) printf("\nSlew to %s aborted. Limits Exceeded or object is below horizon.\n\n", object);
   
		//=================================================================
		close(sock);
		//=================================================================
		sleep(1);	
		//CREATING THE SOCKET
     		sock = socket(AF_INET , SOCK_STREAM , 0);
     	 	if (sock == -1)
       		{
			log_fatal("Could not create socket TCP.");
			report_and_exit("Could not create socket TCP.");
		}
		log_info("Socket TCP created.");
		printf("Socket TCP created.\n");

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
		printf("Connection stablished with the server (%s:%ld)\n\n", IP_SERVER, TCP_PORT);


		// ****************************
		//SEND ABORT DATA
		if (debug_flag) printf("\n");
		for(i = 0; i<stall_data_nlines ; i++)
		{
			if( send(sock , stall_data[i] , strlen(stall_data[i]) , 0) < 0)
			{
				log_fatal("Could not send data to remote socket server.");
				report_and_exit("Could not send data to remote socket server.");
			}
			if (debug_flag) printf("%s\n",stall_data[i]);
		}
		if (debug_flag) printf("\n");

		usleep(TX_DELAY);
		log_info("Stall instructions sent to remote server...");
		if (debug_flag) printf("Stall instructions sent to remote server.\n");
		
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

		//RECEIVE A REPLY FROM THE SERVER
		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
		{
			log_fatal("Could not receive data from socket server.");
			report_and_exit("Could not receive data from socket server.");
		}
		if (debug_flag) printf("Stall reply received from socket server.\n");

		//REPLY DATA ANALYZER 
		// *********************
		if (debug_flag) printf("Server Reply: [%s]\n", server_reply);
		if (debug_flag) printf("Char Counter: [%ld]\n", strlen(server_reply));


        	if ( memcmp(server_reply,"Stall.|No error. Error = 0.", strlen(server_reply)) == 0 )
        	{
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
			if (debug_flag) printf("Operation mode set: %d (STALL)\n\n", opmode_var->opmode);
 			if (debug_flag || verbose_flag) printf("Stall. Tracking off.\n");
		}else{
			if (debug_flag) printf("\nThe condition STALL was not matched.\n");
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
		
		report_and_exit("Normal exit");
        }
	if (debug_flag) printf("Limits not Exceeded.\n");
        
        // Is the telescope found some problems to do the slew? IS PARKED
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if ( memcmp(server_reply,"TypeError: Error, the device is parked and must be unparked using the Unpark command before proceeding. Error = 216.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
	{
		log_warn("Slew Aborted! Telescope is Parked. Try typing -u to UnPark or -h for help.");
                if (debug_flag || verbose_flag) printf("\nSlew to %s aborted. Telescope is Parked. Try typing -u to UnPark or -h for help.\n\n", object);
		// Routine for Op_Mode = PARKED
		// - - - - - - - - - - - - - - -
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
		{	
			opmode_var->object = PARK;
			opmode_var->opmode = PARKED;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag)
		{
			printf("----------------------\n");
			printf("Operation Mode Set:   \n");
			printf("----------------------\n");
			printf("Object = %d (PARK)\n",opmode_var->object);
			printf("OpMode = %d (PARKED)\n",opmode_var->opmode);
			printf("----------------------\n\n");
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
		
		report_and_exit("Normal exit");

	}
	if (debug_flag) printf("\nTelescope not Parked.\n");

        
	// Is the telescope found some problems to do the slew? TELESCOPE NOT HOMED 
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if ( memcmp(server_reply,"TypeError: The operation failed because the mount is not yet homed. Error = 231.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		if (debug_flag || verbose_flag)
		{
			printf("\n\n    * * * TELESCOPE NOT HOMED ! * * *\n\n");

			printf("Would you like to find the home position now? Type <y> for yes, or any key to abort.\n");
			printf("------------------------------------------------------------------------------------\n");
			printf("The telescope mount was probably turned the power on recently, so is necessary\n");
		       	printf("find the home position to start the motors encoders. If you chose to do find home\n");
			printf("now, after that, the slew to %s will be resumed.\n", object);
			printf("------------------------------------------------------------------------------------\n\n");

		}
		system("/bin/stty raw -echo");
		ch = getchar();
		system("/bin/stty cooked echo");
		if (ch != 'y')
		{
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
			if (debug_flag) printf("\nOperation mode set: %d (STALL)\n", opmode_var->opmode);

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
			//
			log_error("Telescope is not homed. Slew aborted by user.");
			if (debug_flag || verbose_flag) printf("Telescope is not homed. Slew aborted by user.\n");
			report_and_exit("Aborted by user.");
		}else{
			//##############################
			// FINDING HOME POSITION ROUTINE
			//

			//=================================================================
			close(sock);
			//=================================================================
			sleep(1);	
			
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
	
			//****************************
			//SEND FIND HOME DATA
			if (debug_flag) printf("\n");
			for(i = 0; i<find_Home_data_nlines ; i++)
			{
				if( send(sock , find_Home_data[i] , strlen(find_Home_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				if (debug_flag) printf("%s\n",find_Home_data[i]);
			}
			if (debug_flag) printf("\n");

			usleep(TX_DELAY);
	
			if (debug_flag || verbose_flag) printf("Finding home position...\n");
			
			// Routine for Op_Mode = SLEW to HOME
			// - - - - - - - - - - - - - - -
			// THIS IS THE OP_MODE ROUTINE - BEGIN
			// Use semaphore as a mutex (lock) by waiting for writer to increment it
			if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
			{
				opmode_var->object = HOME;
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
				printf("Object = %d (HOME)\n",opmode_var->object);
				printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
				printf("----------------------\n\n");
			}
			log_info("Data instructions sent to remote server. Finding home position...");
			if (debug_flag) printf("Find home position data instructions sent to remote server.\n");
			
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

			//RECEIVE A REPLY FROM THE SERVER
			if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
			{
				log_fatal("Could not receive data from socket server.");
				report_and_exit("Could not receive data from socket server.");
			}
			if (debug_flag) printf("Find home position data reply received from socket server.\n");
	
			//REPLY DATA ANALYZER 
			//*********************
			if (debug_flag) printf("Server Reply: [%s]\n", server_reply);
			if (debug_flag) printf("Char Counter: [%ld]\n", strlen(server_reply));


			//VERIFYING IF TELESCOPE IS HOMED
			if ( memcmp(server_reply , "Homed.|No error. Error = 0." , strlen(server_reply)) == 0 )   //Find home complete  
			{
				log_info("Telescope is homed!");
				if (debug_flag || verbose_flag) printf("Finding home position...DONE!\n");
				usleep(TX_DELAY);
				if (debug_flag || verbose_flag) printf("\nResuming the slewing to %s now...\n", object);
				log_info("Resuming the slewing to %s now...", object);

				// Routine for Op_Mode = SLEW to Last Object Before Finding Home
				// - - - - - - - - - - - - - - -
				// THIS IS THE OP_MODE ROUTINE - BEGIN
				// Use semaphore as a mutex (lock) by waiting for writer to increment it
				if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
				{
					opmode_var->object = keywordmap[i].targets;
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
					printf("Object = %d (%s)\n",opmode_var->object, object);
					printf("OpMode = %d (SLEW)\n",opmode_var->opmode);
					printf("----------------------\n\n");
				}

				// Routine for Op_Mode = Slewing
				
				//****************************
				//SEND RESUME POINT AND TRACKING DATA
				if (debug_flag) printf("\n");
				for(i = 0; i<set_pTrack_data_nlines ; i++)
				{
					if( send(sock , set_pTrack_data[i] , strlen(set_pTrack_data[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					if (debug_flag) printf("%s\n",set_pTrack_data[i]);
				}
				if (debug_flag) printf("\n");

				usleep(TX_DELAY);
				log_info("Data instructions sent to remote server.");
				if (debug_flag) printf("Set pTrack data instructions sent to remote server.\n");
				
				
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


				//RECEIVE A REPLY FROM THE SERVER
				if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
				{
					log_fatal("Could not receive data from socket server.");
					report_and_exit("Could not receive data from socket server.");
				}
				if (debug_flag) printf("Set pTrack data received from socket server.\n");

				//REPLY DATA ANALYZER 4
				//*********************
				if (debug_flag) printf("Server Reply: [%s]\n", server_reply);
				if (debug_flag) printf("Char Counter: [%ld]\n", strlen(server_reply));
				
			}else{
				if (debug_flag) printf("\nThe condition HOMED was not matched.\n");
			}
		}
	}else {
		if (debug_flag) printf("Telescope is HOMED.\n");
	}

	// Is the telescope finished the slewing to the object?
	// - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if ( memcmp(server_reply,"isSlew=1.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		log_info("Slew to %s completed. Tracking on %s RaDec rates.", object, object);
		if (debug_flag || verbose_flag) printf("\nSlew to %s completed. Tracking on %s RaDec rates.\n\n", object, object);
	
		// Routine for Op_Mode = Tracking
		//------------------------------------
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))		//Wait until semaphore != 0
		{
			opmode_var->opmode = TRACK;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		if (debug_flag) printf("Operation Mode set: %d (TRACK)\n", opmode_var->opmode);
	}else{
		if (debug_flag || verbose_flag) printf("\nSlew to the %s not completed. Server error!.\n", object);
		
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
	
	if (debug_flag) printf("End of pTrack.\n\n");

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


