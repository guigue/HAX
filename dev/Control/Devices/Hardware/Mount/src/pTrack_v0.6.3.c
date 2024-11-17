/*
        =================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        =================================================================================

        Point and Track v.0.6.3
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
	  	|		|
	  0.6.4 |		|
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
		printf ("\n======== DEBUG BEGIN =======\n\nnon-option ARGV-elements: ");
		while (optind < argc)
			printf ("%s \n", argv[optind++]);
		if (diff > 1)
		{
			printf("\n");
			puts("pTrack: Too many arguments for object. Try -h or --help.");
			exit(1);
		}
		printf("\rObject = %s                 \n",object);
	}
	
	// Getting flag from Special Object Arguments
	// -------------------------------------------
	if (object)
	{
		if (memcmp(object,"PARK",4)       == 0)	park_flag = 1;
		if (memcmp(object,"UNPARK",6)     == 0)	unpark_flag = 1;
		if (memcmp(object,"CONNECT",7)    == 0)	connect_flag = 1;
		if (memcmp(object,"DISCONNECT",10) == 0)	disconnect_flag = 1;
		if (memcmp(object,"HOME",4)       == 0)	findhome_flag = 1;
	}
	//--------------------------------------------



	printf("optind = %d\n",optind);
	printf("argc = %d\n",argc);
	printf("optflag_ctr = %d\n",optflag_ctr);




	//Resume of options activated
	if(version_flag)	printf("version_flag = %d\n", version_flag);
	if(debug_flag)		printf("debug_flag = %d\n", debug_flag);
	if(opmode_flag)		printf("opmode_flag = %d\n", opmode_flag);
	if(joy_flag) 		printf("joy_flag = %d\n", joy_flag);

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


	printf("\nRun the pTrack instructions!\n\n");


	if (help_flag)
	{
		print_usage();
		//log_warn("Bad parameter when enter pTrack instructions.");
        }


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





	// DECLARACAO VARIAVEIS DA ROTINA DE COMUNICACAO COM SERVIDOR
	// ----------------------------------------------------------

        int i,j                                 ;       //Generic counter
        int sock                                ;       //Socket variable
        struct sockaddr_in server               ;       //Socket variable
        char server_reply[RCV_BUFFER_SIZE]      ;       //Socket variable
//	char server_reply2[RCV_BUFFER_SIZE]     ;       //Socket variable

	// Javascript instructions for TheSkyX server ( Object Verification Instruction )
	// ------------------------------------------
	char find_object_data[7][50]           ;       // Check if object exists in the server [Line][Colum].
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
	char find_Home_data[9][50]              ;       //Instructions to find home
	int find_Home_data_nlines = 9           ;       //Getter commands number of lines

	sprintf(find_Home_data[0],"/* Java Script */			");
	sprintf(find_Home_data[1],"/* Socket Start Packet */		");
	sprintf(find_Home_data[2],"var Out;				");
	sprintf(find_Home_data[3],"if (!sky6RASCOMTele.IsConnected){	");
	sprintf(find_Home_data[4],"	sky6RASCOMTele.Connect();	");
	sprintf(find_Home_data[5],"	sky6RASCOMTele.FindHome();	");
	sprintf(find_Home_data[6],"	Out='Homed.';}			");
	sprintf(find_Home_data[7],"Out;					");
	sprintf(find_Home_data[8],"/* Socket End Packet */		");




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


	// Javascript instructions for TheSkyX server (ABORT)
	// ------------------------------------------
	char abort_pTrack_data[13][50]          ;       //Instructions to Abort
	int abort_pTrack_data_nlines = 13       ;       //Getter commands number of lines

	sprintf(abort_pTrack_data[0],"/* Java Script */                         ");
	sprintf(abort_pTrack_data[1],"/* Socket Start Packet */                 ");
	sprintf(abort_pTrack_data[2],"var Out;                                  ");
	sprintf(abort_pTrack_data[3],"var err;                                  ");
	sprintf(abort_pTrack_data[4],"sky6StarChart.LASTCOMERROR = 0;           ");
	sprintf(abort_pTrack_data[5],"sky6RASCOMTele.SetTracking(0,1,0,0);      ");
	sprintf(abort_pTrack_data[6],"err = sky6StarChart.LASTCOMERROR;         ");
	sprintf(abort_pTrack_data[7],"if (err != 0){                            ");
	sprintf(abort_pTrack_data[8],"  Out = 'Erro.';                          ");
	sprintf(abort_pTrack_data[9],"}else{                                    ");
	sprintf(abort_pTrack_data[10]," Out = 'Aborted.';                       ");
	sprintf(abort_pTrack_data[11],"}                                        ");
	sprintf(abort_pTrack_data[12],"/* Socket End Packet */                  ");




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
		PARK,"PARK",                    //20
		UNPARK,"UNPARK",                //21
		HOME,"HOME",                    //22
		CONNECT,"CONNECT",              //23
		DISCONNECT,"DISCONNECT",        //24
		EXTRA_OBJ, "extra object"       //30
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






	//==============================================================
	//
	//  BEGIN OF INICIAL VERIFICATION ROUTINES 
	//
	//  ( CONNECT, DISCONNECT, PARK, UNPARK )
	//
	//
	//==============================================================
	
	if (!object || (memcmp(object,"PARK",4) == 0) || (memcmp(object,"UNPARK",6) == 0) || (memcmp(object,"CONNECT",7) == 0) || (memcmp(object,"DISCONNECT",10) == 0) || (memcmp(object,"HOME",4) == 0))
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



		//SENDING DATA INSTRUCTIONS TO SERVER BASED ON OPTIONS
		/************************************************************/
      		//------------------------------------------------------------
		
		if ((connect_flag == 1 && park_flag == 0) || (connect_flag == 1 && unpark_flag == 1))
		{
			for(i = 0; i<connect_data_nlines ; i++)
      			{
				if( send(sock , connect_data[i] , strlen(connect_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
       		 	}
       	 		usleep(TX_DELAY);
       			log_info("Connect data instructions sent to remote server.");
			printf("Connect data instructions sent to remote server.\n");
		}

		if (connect_flag == 1 && park_flag == 1)
		{
			for(i = 0; i<connectP_data_nlines ; i++)
	      		{
				if( send(sock , connectP_data[i] , strlen(connectP_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
       	 		}
       		 	usleep(TX_DELAY);
       			log_info("Connect and Park data instructions sent to remote server.");
			printf("Connect and Park data instructions sent to remote server.\n");
		}

		if (disconnect_flag == 1 && park_flag == 0)
		{
			for(i = 0; i<disconnect_data_nlines ; i++)
      			{
				if( send(sock , disconnect_data[i] , strlen(disconnect_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
       		 	}
        		usleep(TX_DELAY);
   	    		log_info("Disconnect data instructions sent to remote server.");
			printf("Disconnect data instructions sent to remote server.\n");
		}

		if (park_flag == 1 && disconnect_flag == 0 && connect_flag == 0)
		{
			for(i = 0; i<park_data_nlines ; i++)
			{
				if( send(sock , park_data[i] , strlen(park_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
       		 	}
        		usleep(TX_DELAY);
       			log_info("Park and Do Not Disconnect data instructions sent to remote server.");
			printf("Park and Do Not Disconnect data instructions sent to remote server.\n");
		}

		if (park_flag == 1 && disconnect_flag == 1)
		{
			for(i = 0; i<parkD_data_nlines ; i++)
      			{
				if( send(sock , parkD_data[i] , strlen(parkD_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
       		 	}
        		usleep(TX_DELAY);
       			log_info("Park and Disconnect data instructions sent to remote server.");
			printf("Park and Disconnect data instructions sent to remote server.\n");
		}
	
		if (unpark_flag == 1 && connect_flag == 0)
		{
			for(i = 0; i<unpark_data_nlines ; i++)
      			{
				if( send(sock , unpark_data[i] , strlen(unpark_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
        		}
        		usleep(TX_DELAY);
       			log_info("UnPark data instructions sent to remote server.");
			printf("UnPark data instructions sent to remote server.\n");
		}
	
		if (findhome_flag)
		{
			for(i = 0; i<find_Home_data_nlines ; i++)
      			{
				if( send(sock , find_Home_data[i] , strlen(find_Home_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
        		}
        		usleep(TX_DELAY);
       			log_info("Find Home data instructions sent to remote server.");
			printf("Find Home data instructions sent to remote server.\n");
		}
		//------------------------------------------------------------
    	
	
	
		printf("\n----------------------------------\n");
		printf("Cleanning server_reply variable...\n");
		strncpy(server_reply, "", sizeof(server_reply));
		printf("Server Reply: [%s]\n", server_reply);
		printf("Char Counter: [%ld]\n", strlen(server_reply));
		printf("----------------------------------\n\n");
	


		//RECEIVE A REPLY FROM THE SERVER ABOUT VERIFICATION DATA
    	  	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
   		{
			log_fatal("Could not receive data from socket server.");
               		report_and_exit("Could not receive data from socket server.");
		}
		//-------------------------------------------------------
		printf("Data received from socket server.\n");

		//Displaying information from server
		printf("Server Reply: [%s]\n", server_reply);
		printf("Char Counter: [%ld]\n", strlen(server_reply));


		// REPLY FROM CONNECT_DATA
		if ( memcmp(server_reply,"Connected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope Connected.");
			printf("\r\nTelescope Connected.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyConnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Connected.");
			printf("\r\nTelescope Already Connected.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
		}

		// REPLY FROM CONNECTP_DATA
		if ( memcmp(server_reply,"Connected.Parked.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope Connected and Parked.");
			printf("\r\nTelescope Connected and Parked.\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyConnected.Parked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Connected and Parked.");
			printf("\r\nTelescope Already Connected and Parked.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyConnected.AlreadyParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Connected and Already Parked.");
			printf("\r\nTelescope Already Connected and Already Parked.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
		}

		// REPLY FROM DISCONNECT_DATA
		if ( memcmp(server_reply,"Disconnected.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope Disconnected.");
			printf("\r\nTelescope Disconnected.\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyDisconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Disconnected.");
			printf("\r\nTelescope Already Disconnected.\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
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
			printf("Operation mode set: %d (PARKED)\n\n", opmode_var->opmode);
       	 
			log_info("Telescope is Parked and Not Disconnected.");
			printf("\r\nTelescope is Parked and Not Disconnected.\n");
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Parked.");
			printf("\r\nTelescope is Already Parked.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
		
		}else if ( memcmp(server_reply,"AlreadyIsDisconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Disconnected.");
			printf("\r\nTelescope is Already Disconnected.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
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
			printf("Operation mode set: %d (PARKED)\n\n", opmode_var->opmode);
        
			log_info("Telescope is Parked and Disconnected.");
			printf("\r\nTelescope is Parked and Disconnected.\n");
		
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyParked.Disconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Parked and Disconnected.");
			printf("\r\nTelescope is Already Parked and Disconnected.\n");
	
			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyDisconnected.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope Already Disconnected.");
			printf("\r\nTelescope Already Disconnected.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
		}

		// REPLY FROM UNPARK_DATA
		if ( memcmp(server_reply,"NotParked.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope was UnParked.");
			printf("\r\nTelescope was UnParked.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
	
		}else if ( memcmp(server_reply,"AlreadyNotParked.|No error. Error = 0.", strlen(server_reply)) == 0 )
		{
			log_info("Telescope is Already Not Parked.");
			printf("\r\nTelescope is Already Not Parked.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			report_and_exit("Normal exit");
		}
		
		// REPLY FROM FIND_HOME_DATA
		if ( memcmp(server_reply,"Homed.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
		{
			log_info("Telescope was Homed.");
			printf("\r\nTelescope was Homed.\n");

			//=================================================================
			close(sock);
			//=================================================================
			log_info("Socket TCP closed.");
			printf("Socket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
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

	char ch;

	//CODE BELOW TRANSFERED TO PTRACK.H
/*	typedef struct 
	{
		int targets;
		char * keyword;
	} target_map ;
	
	target_map keywordmap[] = {
		SKY,"sky",			//0
		MERCURY,"mercury",		//1
		VENUS,"venus",			//2
		EARTH,"earth",			//3
		MARS,"mars",			//4
		JUPITER,"jupiter",		//5
		SATURN,"saturn",		//6
		URANUS,"uranus",		//7
		NEPTUNE,"neptune",		//8
		PLUTO,"pluto",			//9
		MOON,"moon",			//10
		SUN,"sun",			//11
		AR,"AR",			//12
		PARK,"PARK",			//20
		UNPARK,"UNPARK",		//21
		HOME,"HOME",			//22
		CONNECT,"CONNECT",		//23
		DISCONNECT,"DISCONNECT",	//24
		EXTRA_OBJ, "extra object"	//30
	};
*/

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
			printf("Operation mode Extra Object set: %d (%s) ->argument:'%s'\n", opmode_var->object, keywordmap[i].keyword, object);
		}
	}

	// Saving the regular object in the Shared Memory for Operation Mode (by index "i")

	if (!sem_wait(semptr_opmode))		//Wait until semaphore != 0
	{
		opmode_var->object = keywordmap[i].targets;
		sem_post(semptr_opmode);
	}
	printf("Operation mode object set: %d (%s)\n", opmode_var->object, keywordmap[i].keyword);



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



	
	//FirstTry If telescope is Parked but have an option to UnPark
	// Example:  ./pTrack -u sun
	//
	if (unpark_flag)
	{
		for(i = 0; i<unpark_data_nlines ; i++)
        	{
			if( send(sock , unpark_data[i] , strlen(unpark_data[i]) , 0) < 0)
			{
				log_fatal("Could not send data to remote socket server.");
				report_and_exit("Could not send data to remote socket server.");
			}
        	}
        	usleep(TX_DELAY);
       		log_info("UnPark data instructions sent to remote server.");
		printf("UnPark data instructions sent to remote server.\n");
		

		//RECEIVE A REPLY FROM THE SERVER ABOUT VERIFICATION DATA
      		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        	{
			log_fatal("Could not receive data from socket server.");
        	       	report_and_exit("Could not receive data from socket server.");
		}
		//-------------------------------------------------------
		printf("UnPark data received from socket server.\n");

		printf("\n----------------------------------\n");
		printf("Cleanning server_reply variable...\n");
		strncpy(server_reply, "", sizeof(server_reply));
		printf("Server Reply: [%s]\n", server_reply);
		printf("Char Counter: [%ld]\n", strlen(server_reply));
		printf("----------------------------------\n\n");
	
	}




	//SENDING DATA TO SERVER
	/************************************************************/
      	//SEND VERIFICATION DATA
      	//------------------------------------------------------------
	
	for(i = 0; i<find_object_data_nlines ; i++)
        {
		if( send(sock , find_object_data[i] , strlen(find_object_data[i]) , 0) < 0)
		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
        }
        usleep(TX_DELAY);
       	log_info("Verification data instructions sent to remote server.");
	printf("Verification data instructions sent to remote server.\n");
	//------------------------------------------------------------
	
    	
	//RECEIVE A REPLY FROM THE SERVER ABOUT VERIFICATION DATA
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
		log_fatal("Could not receive data from socket server.");
               	report_and_exit("Could not receive data from socket server.");
	}
	//-------------------------------------------------------
	printf("Verification data received from socket server.\n");


	//REPLY DATA ANALYZER 1 - Is the object found at server?
	//****************************************************
	//int reply_cut1_ctr = count_char(server_reply, '|');	//Count a string with a char limiter	
	//char reply_cut1[reply_cut1_ctr];			//Creat a fixed string
	//copy_char ( server_reply, reply_cut1,'|');		//Copy a string with a char limiter
	//int server_reply_ctr = (strlen(server_reply)-21);
	//char reply_cut1[server_reply_ctr];			//Creat a fixed string
	//copy_char ( server_reply, reply_cut1,'|');		//Copy a string with a char limiter
	//memcpy(reply_cut1, server_reply, server_reply_ctr);
	printf("Server Reply: [%s]\n", server_reply);
	//printf("Server Reply Cutted: [%s]\n", reply_cut1);
	printf("Char Counter: [%ld]\n", strlen(server_reply));
	//printf("Reply Cutted: [%s]\n", reply_cut1);

	if ( memcmp(server_reply,"TypeError: Object not found. Error = 250.|No error. Error = 0.", strlen(server_reply)) == 0 )        
	{
		log_error("Target '%s' not found.", object );
		printf("\r\nTarget '%s' not found. Try another object or verify for sintax error from object name.\n", object);
		//=================================================================
		close(sock);
		//=================================================================
		log_info("Socket TCP closed.");
		printf("Socket TCP closed.\n");
		//Cleanup Op_Mode Shared Memory Stuff
		munmap(opmode_var, ByteSize_opmode);
		close(fd_shmem_opmode);
		sem_close(semptr_opmode);
		printf("Shared Memory cleanup ok.\n");
		//---------------------------------------------------------------
		fclose(fp);
		printf("File descriptor fp closed.\n");
		//
		report_and_exit("Normal exit");
	
	}else if ( memcmp(server_reply,"NG|Another socket command is still in progress. Error = 219", strlen(server_reply)) == 0 )
	{
		printf("\r\nAnother socket command is still in progress.\n");
		log_error("Another socket command is still in progress.");
		
		//=================================================================
		close(sock);
		//=================================================================
		log_info("Socket TCP closed.");
		printf("Socket TCP closed.\n");
		//Cleanup Op_Mode Shared Memory Stuff
		munmap(opmode_var, ByteSize_opmode);
		close(fd_shmem_opmode);
		sem_close(semptr_opmode);
		printf("Shared Memory cleanup ok.\n");
		//---------------------------------------------------------------
		fclose(fp);
		printf("File descriptor fp closed.\n");
		//
		report_and_exit("Normal exit");
				
	}else if ( memcmp(server_reply,"Object found.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{	
		printf("\r\nObject found. Target to %s selected. Slewing...\n", object);
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
		printf("Operation mode set: %d (SLEW)\n\n", opmode_var->opmode);
				//****************************
      		//SEND POINT AND TRACKING DATA
      		for(i = 0; i<set_pTrack_data_nlines ; i++)
       	 	{
			if( send(sock , set_pTrack_data[i] , strlen(set_pTrack_data[i]) , 0) < 0)
       	    		{
			log_fatal("Could not send data to remote socket server.");
			report_and_exit("Could not send data to remote socket server.");
		}
			usleep(TX_DELAY);
		}
	      	 	log_info("Set pTrack Data instructions sent to remote server.");
       	 	printf("Set pTrack Data instructions sent to remote server.\n");

	}else
	{
		log_error("Server Reply is strange: [%s]", server_reply);
		printf("Server Reply is strange: [%s]\n", server_reply);
		report_and_exit("Exit");
	}
	//-----------------------------------------------------

	printf("\n----------------------------------\n");
	printf("Cleanning server_reply variable...\n");
	strncpy(server_reply, "", sizeof(server_reply));
	printf("Server Reply: [%s]\n", server_reply);
	printf("Char Counter: [%ld]\n", strlen(server_reply));
	printf("----------------------------------\n\n");
	
      	//RECEIVE A REPLY FROM THE SERVER
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
		log_fatal("Could not receive data from socket server.");
               	report_and_exit("Could not receive data from socket server.");
	}
	printf("Set pTrack data received from socket server.\n");

	//REPLY DATA ANALYZER 2 - Fast Problems before slewing
	//*********************
	//int reply_cut2_ctr = count_char(server_reply, '|');	//Count a string with a char limiter
	//char reply_cut2[reply_cut2_ctr];			//Creat a fixed string
	//copy_char ( server_reply, reply_cut2,'|');		//Copy a string with a char limiter
	printf("Server Reply: [%s]\n", server_reply);
	printf("Char Counter: [%ld]\n", strlen(server_reply));
	
	if ( memcmp(server_reply,"TypeError: A Mount command is already in progress. Error = 121.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		printf("\r\nAnother mount command is already in progress.\n");
		log_error("Another mount command is already in progress.");
		
		//=================================================================
		close(sock);
		//=================================================================
		log_info("Socket TCP closed.");
		printf("Socket TCP closed.\n");
		//Cleanup Op_Mode Shared Memory Stuff
		munmap(opmode_var, ByteSize_opmode);
		close(fd_shmem_opmode);
		sem_close(semptr_opmode);
		printf("Shared Memory cleanup ok.\n");
		//---------------------------------------------------------------
		fclose(fp);
		printf("File descriptor fp closed.\n");
		//
		report_and_exit("Normal exit");
	}

        // Is the telescope found some problems to do the slew? LIMITS EXCEEDED
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if ( memcmp(server_reply,"TypeError: Limits exceeded. Error = 218.|No error. Error = 0.", strlen(server_reply)) == 0 )
        {
                log_warn("Slew Aborted! Limits Exceeded or object is below horizon.");
                printf("\nSlew to %s aborted. Limits Exceeded or object is below horizon.\n\n", object);
/*   
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

*/
		// ****************************
		//SEND ABORT DATA
		for(i = 0; i<abort_pTrack_data_nlines ; i++)
		{
			if( send(sock , abort_pTrack_data[i] , strlen(abort_pTrack_data[i]) , 0) < 0)
			{
				log_fatal("Could not send data to remote socket server.");
				report_and_exit("Could not send data to remote socket server.");
			}
			usleep(TX_DELAY);
		}
		log_info("Aborting instructions sent to remote server...");
		printf("Aborting instructions sent to remote server.\n");
		
		printf("\n----------------------------------\n");
		printf("Cleanning server_reply variable...\n");
		strncpy(server_reply, "", sizeof(server_reply));
		printf("Server Reply: [%s]\n", server_reply);
		printf("Char Counter: [%ld]\n", strlen(server_reply));
		printf("----------------------------------\n\n");

		//RECEIVE A REPLY FROM THE SERVER
		if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
		{
			log_fatal("Could not receive data from socket server.");
			report_and_exit("Could not receive data from socket server.");
		}
		printf("Aborting reply received from socket server.\n");

		//REPLY DATA ANALYZER 
		// *********************
		//int reply_cut3_ctr = count_char(server_reply2, '|');	//Count a string with a char limiter
		//char reply_cut3[reply_cut3_ctr];			//Creat a fixed string
		//copy_char ( server_reply2, reply_cut3,'|');		//Copy a string with a char limiter
		printf("Server Reply: [%s]\n", server_reply);
		printf("Char Counter: [%ld]\n", strlen(server_reply));


        	if ( memcmp(server_reply,"Aborted.|No error. Error = 0.", strlen(server_reply)) == 0 )
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
			printf("Operation mode set: %d (STALL)\n\n", opmode_var->opmode);
        	}else{
			printf("\nThe condition ABORTED was not matched.\n");
		}

		//=================================================================
		close(sock);
		//=================================================================
		log_info("Socket TCP closed.");
		printf("Socket TCP closed.\n");
		//Cleanup Op_Mode Shared Memory Stuff
		munmap(opmode_var, ByteSize_opmode);
		close(fd_shmem_opmode);
		sem_close(semptr_opmode);
		printf("Shared Memory cleanup ok.\n");
		//---------------------------------------------------------------
		fclose(fp);
		
		printf("File descriptor fp closed.\n");
		
		report_and_exit("Normal exit");
        }
	printf("Limits not Exceeded.\n");
        
        // Is the telescope found some problems to do the slew? IS PARKED
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        if ( memcmp(server_reply,"TypeError: Error, the device is parked and must be unparked using the Unpark command before proceeding. Error = 216.|No error. Error = 0.", strlen(server_reply)) == 0 ) 
	{
		log_warn("Slew Aborted! Telescope is Parked. Try typing -u to UnPark or -h for help.");
                printf("\nSlew to %s aborted. Telescope is Parked. Try typing -u to UnPark or -h for help.\n\n", object);
		// Routine for Op_Mode = PARKED
		// - - - - - - - - - - - - - - -
		// THIS IS THE OP_MODE ROUTINE - BEGIN
		// Use semaphore as a mutex (lock) by waiting for writer to increment it
		if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
		{
			opmode_var->opmode = PARKED;
			sem_post(semptr_opmode);
		}
		// THIS IS THE OP_MODE ROUTINE = END
		// ---------------------------------
		printf("Operation mode set: %d (PARKED)\n\n", opmode_var->opmode);
        
		//=================================================================
		close(sock);
		//=================================================================
		log_info("Socket TCP closed.");
		printf("Socket TCP closed.\n");
		//Cleanup Op_Mode Shared Memory Stuff
		munmap(opmode_var, ByteSize_opmode);
		close(fd_shmem_opmode);
		sem_close(semptr_opmode);
		printf("Shared Memory cleanup ok.\n");
		//---------------------------------------------------------------
		fclose(fp);
		
		printf("File descriptor fp closed.\n");
		
		report_and_exit("Normal exit");

	}
	printf("\nTelescope not Parked.\n");

        
	// Is the telescope found some problems to do the slew? TELESCOPE NOT HOMED 
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if ( memcmp(server_reply,"TypeError: The operation failed because the mount is not yet homed. Error = 231.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		printf("\n\n    * * * TELESCOPE NOT HOMED ! * * *\n\n");

		printf("Would you like to find the home position now? Type <y> for yes, or any key to abort.\n");
		printf("------------------------------------------------------------------------------------\n");
		printf("The telescope mount was probably turned the power on recently, so is necessary\n");
	       	printf("find the home position to start the motors encoders. If you chose to do find home\n");
		printf("now, after that, the slew to %s will be resumed.\n", object);
		printf("------------------------------------------------------------------------------------\n\n");

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
			printf("\nOperation mode set: %d (STALL)\n", opmode_var->opmode);

			//=================================================================
			close(sock);
 			//=================================================================
			log_info("Socket TCP closed.");
			printf("\nSocket TCP closed.\n");
			//Cleanup Op_Mode Shared Memory Stuff
			munmap(opmode_var, ByteSize_opmode);
			close(fd_shmem_opmode);
			sem_close(semptr_opmode);
			printf("Shared Memory cleanup ok.\n");
			//---------------------------------------------------------------
			fclose(fp);
			printf("File descriptor fp closed.\n");
			//
			log_error("Telescope is not homed. Slew aborted by user.");
			printf("Telescope is not homed. Slew aborted by user.\n");
			report_and_exit("Aborted by user.");
		}else{
			printf("Finding home position...\n");
			
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
	
			//****************************
			//SEND FIND HOME DATA
			for(i = 0; i<find_Home_data_nlines ; i++)
			{
				if( send(sock , find_Home_data[i] , strlen(find_Home_data[i]) , 0) < 0)
				{
					log_fatal("Could not send data to remote socket server.");
					report_and_exit("Could not send data to remote socket server.");
				}
				usleep(TX_DELAY);
			}
			log_info("Data instructions sent to remote server. Finding home position...");
			printf("Find home position data instructions sent to remote server.\n");
			
			printf("\n----------------------------------\n");
			printf("Cleanning server_reply variable...\n");
			strncpy(server_reply, "", sizeof(server_reply));
			printf("Server Reply: [%s]\n", server_reply);
			printf("Char Counter: [%ld]\n", strlen(server_reply));
			printf("----------------------------------\n\n");
	
			//RECEIVE A REPLY FROM THE SERVER
			if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
			{
				log_fatal("Could not receive data from socket server.");
				report_and_exit("Could not receive data from socket server.");
			}
			printf("Find home position data reply received from socket server.\n");
	
			//REPLY DATA ANALYZER 
			//*********************
			//int reply_cut3_ctr = count_char(server_reply2, '|');	//Count a string with a char limiter
			//char reply_cut3[reply_cut3_ctr];			//Creat a fixed string
			//copy_char ( server_reply2, reply_cut3,'|');		//Copy a string with a char limiter
			printf("Server Reply: [%s]\n", server_reply);
			printf("Char Counter: [%ld]\n", strlen(server_reply));


			//VERIFYING IF TELESCOPE IS HOMED
			if ( memcmp(server_reply , "Homed.|No error. Error = 0." , strlen(server_reply)) == 0 )   //Find home complete  
			{
				log_info("Telescope is homed!");
				printf("Finding home position...DONE!\n");
				usleep(TX_DELAY);
				printf("\nResuming the slewing to %s now...\n", object);
				log_info("Resuming the slewing to %s now...", object);

				// Routine for Op_Mode = Slewing
				// - - - - - - - - - - - - - - -
				// THIS IS THE OP_MODE ROUTINE - BEGIN
				// Use semaphore as a mutex (lock) by waiting for writer to increment it
				if (!sem_wait(semptr_opmode))           //Wait until semaphore != 0
				{
					opmode_var->opmode = SLEW;
					sem_post(semptr_opmode);
				}
				printf("Operation mode set: %d (SLEW)\n", opmode_var->opmode);
				// THIS IS THE OP_MODE ROUTINE = END
				// ---------------------------------
				//****************************
				//SEND RESUME POINT AND TRACKING DATA
				for(i = 0; i<set_pTrack_data_nlines ; i++)
				{
					if( send(sock , set_pTrack_data[i] , strlen(set_pTrack_data[i]) , 0) < 0)
					{
						log_fatal("Could not send data to remote socket server.");
						report_and_exit("Could not send data to remote socket server.");
					}
					usleep(TX_DELAY);
				}
				log_info("Data instructions sent to remote server.");
				printf("Set pTrack data instructions sent to remote server.\n");
				
			
				printf("\n----------------------------------\n");
				printf("Cleanning server_reply variable...\n");
				strncpy(server_reply, "", sizeof(server_reply));
				printf("Server Reply: [%s]\n", server_reply);
				printf("Char Counter: [%ld]\n", strlen(server_reply));
				printf("----------------------------------\n\n");
	

				//RECEIVE A REPLY FROM THE SERVER
				if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
				{
					log_fatal("Could not receive data from socket server.");
					report_and_exit("Could not receive data from socket server.");
				}
				printf("Set pTrack data received from socket server.\n");

				//REPLY DATA ANALYZER 4
				//*********************
				//int reply_cut4_ctr = count_char(server_reply, '|');     //Count a string with a char limiter
				//char reply_cut4[reply_cut4_ctr];                        //Creat a fixed string
				//copy_char ( server_reply, reply_cut4,'|');              //Copy a string with a char limiter
				//printf("Resposta do servidor: [%s]\n",reply_cut4);	
				printf("Server Reply: [%s]\n", server_reply);
				printf("Char Counter: [%ld]\n", strlen(server_reply));
				
			}else{
				printf("\nThe condition HOMED was not matched.\n");
			}
		}
	}else {
		printf("Telescope is HOMED.\n");
	}

	// Is the telescope finished the slewing to the object?
	// - - - - - - - - - - - - - - - - - - - - - - - - - - -
	if ( memcmp(server_reply,"isSlew=1.|No error. Error = 0.", strlen(server_reply)) == 0 )
	{
		log_info("Slew to %s completed. Tracking on %s RaDec rates.", object, object);
		printf("\nSlew to %s completed. Tracking on %s RaDec rates.\n", object, object);
	
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
		printf("Operation Mode set: %d (TRACK)\n", opmode_var->opmode);
	}else{
		printf("\nThe condition isSlew=1 was not matched.\n");
	}

	//=================================================================
      	close(sock);
      	//=================================================================
	log_info("Socket TCP closed.");
	printf("\nSocket TCP closed.\n");
	

	//Cleanup Op_Mode Shared Memory Stuff
	munmap(opmode_var, ByteSize_opmode);
	close(fd_shmem_opmode);
	sem_close(semptr_opmode);
	printf("Shared Memory cleanup ok.\n");
	//---------------------------------------------------------------


	fclose(fp);
	printf("File descriptor fp closed.\n");
	
	printf("End of pTrack.\n\n");

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


