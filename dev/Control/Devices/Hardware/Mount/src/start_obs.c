/*
        =================================================================================
        	           Universidade Presbiteriana Mackenzie
        	Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        =================================================================================

	Start Observation v.0.1
        ---------------------------------------------------------------------------------
        Este programa visa verificar e enviar instrucoes para o TheSkyX de modo a
	conectar o servidor ao mount do telescopio, deixando apto a iniciar os 
	apontamentos e tracking dos objetos. Codigo iniciado tendo como base o pTrack 
	versao 0.4
	
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
          0.1   |  15-02-2021   | Primeira versão, realiza a conecao com o mount.
                |               | 
        ---------------------------------------------------------------------------------
	  	|     		| 
                |               | 
	---------------------------------------------------------------------------------
	  	|  		| 
		|		| 
	---------------------------------------------------------------------------------
	  	|     		| 
		|               | 
	---------------------------------------------------------------------------------
	  	|  		| 
		|		| 
	---------------------------------------------------------------------------------
                |               | 
                |               | 
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



	---------------------------------------------------------------------------------
        Usage:

        # start_obs 

        --------------------------------------------------------------------------------

*/


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <stdint.h>
#include <inttypes.h>

//#include <semaphore.h>  //For Shared Memory
//#include <sys/mman.h>   //For Shared Memory
//#include <sys/stat.h>   //For Shared Memory

#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <fcntl.h>      //open(sock) or file
#include <unistd.h>     //close(sock) or write to a file descriptor
#include <time.h>       //usleep() to timing socket message transfer


#include "log.h"	//Used to logging routine in the end of this code
#include <stdarg.h>	//Used to logging routine in the end of this code
#include "pTrack.h"	//Definitions for pTrack
#include "confuse.h"	//Configuration file support (see: ../etc/HAX_Control.config)



        /***
        *     P R E L I M I N A R Y     F U N C T I O N S
        *****************************************************************/


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

        if (argc != 2)
        {
                printf("\n\tUsage: pTrack <object> \n");
                printf("\t\tobject: Sun, Moon, Jupiter or Venus.\n\n");
		log_warn("Bad parameter when enter pTrack instructions.");
                exit(0);
        }


	// CARREGA O ARQUIVO DE CONFIGURAÇÃO
	// ---------------------------------
        static char *IP_SERVER 			= NULL		;
        static long int TCP_PORT 		= 0		;
        static long int RCV_BUFFER_SIZE 	= 0		;
        static long int TX_DELAY 		= 0		;

	static char *BackingFile		= NULL		;
	static char *SemaphoreName		= NULL		;
	static long int AccessPerms		= 0		;

	static long int RINGSIZE		= 0		;
	static char *DATAFILENAME		= NULL		;

        static char *DIRECTORY_LOG 		= NULL		;


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
	//---------------------------------------------



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

        int i                                   ;       //Generic counter
        int sock                                ;       //Socket variable
        struct sockaddr_in server               ;       //Socket variable
        char server_reply[RCV_BUFFER_SIZE]      ;       //Socket variable
        int command_nlines = 28                 ;       // Getter commands number of lines from set_<object>_data[].
                                                        // If you add a command remember to modify, add, etc a new
                                                        // command_nlines

//	char * p;
//	const char sep[2] =";";
	//  double jd , sid , alt, az , ra , dec , tra , tdec;

//	unsigned long long rb_ctr = 1                         ;       //Ring Buffer Counter


/*	pos_data_type * rbpos_base_ptr, * rbpos_ptr ;
	size_t ByteSize = sizeof(pos_data_type) * RINGSIZE    ;       //Used for Shared Memory and to Write in file
	int fd_shmem                                          ;       //Used for Shared Memory
	sem_t * semptr                                        ;       //Used for Shared Memory

	int fd_data, fd_data_w                                ;       //Used to Open and Write a File
	char *filename = DATAFILENAME                         ;       //Used to Open/Create a File
*/


	// Javascript instructions for TheSkyX server
	// ------------------------------------------

	char* set_Sun_data[] =
	{
		"/* Java Script */								",
		"/* Socket Start Packet */							",
		"var Out;									",
		"var err;									",
		"var obj='Sun';									",
		"sky6StarChart.LASTCOMERROR = 0;						",
		"sky6StarChart.Find(obj);							",
		"err = sky6StarChart.LASTCOMERROR;						",
		"if (err != 0){									",
		"        Out = Target + ' not found.';						",
		"}else{										",
		"        sky6ObjectInformation.Property(54);					",
		"        var targetRA = sky6ObjectInformation.ObjInfoPropOut;			",
		"        sky6ObjectInformation.Property(55);					",
		"        var targetDec = sky6ObjectInformation.ObjInfoPropOut;			",
		"        sky6ObjectInformation.Property(77);					",
		"        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;		",
		"        sky6ObjectInformation.Property(78);					",
		"        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;		",
		"        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj);			",
		"        var slewComplete = sky6Web.IsSlewComplete;				",
		"        while(slewComplete != 1){						",
		"                slewComplete = sky6Web.IsSlewComplete;				",
		"        }									",
		"        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);	",
		"}										",
		"Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';		",
		"/* Socket End Packet */							"
	};

        char* set_Moon_data[] =
        {
                "/* Java Script */                                                              ",
                "/* Socket Start Packet */                                                      ",
                "var Out;                                                                       ",
                "var err;									",
		"var obj='Moon';                                                                ",
                "sky6StarChart.LASTCOMERROR = 0;                                                ",
                "sky6StarChart.Find(obj);                                                       ",
                "err = sky6StarChart.LASTCOMERROR;                                              ",
                "if (err != 0){                                                                 ",
                "        Out = Target + ' not found.';                                          ",
                "}else{                                                                         ",
                "        sky6ObjectInformation.Property(54);                                    ",
                "        var targetRA = sky6ObjectInformation.ObjInfoPropOut;                   ",
                "        sky6ObjectInformation.Property(55);                                    ",
                "        var targetDec = sky6ObjectInformation.ObjInfoPropOut;                  ",
                "        sky6ObjectInformation.Property(77);                                    ",
                "        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;            ",
                "        sky6ObjectInformation.Property(78);                                    ",
                "        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;           ",
                "        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj); 		        ",
                "        var slewComplete = sky6Web.IsSlewComplete;                             ",
                "        while(slewComplete != 1){                                              ",
                "                slewComplete = sky6Web.IsSlewComplete;                         ",
                "        }                                                                      ",
                "        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);   ",
                "}                                                                              ",
                "Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';               ",
                "/* Socket End Packet */                                                        "
        };

        char* set_Jupiter_data[] =
        {
                "/* Java Script */                                                              ",
                "/* Socket Start Packet */                                                      ",
                "var Out;                                                                       ",
                "var err;                                                                       ",
                "var obj='Jupiter';                                                             ",
                "sky6StarChart.LASTCOMERROR = 0;                                                ",
                "sky6StarChart.Find(obj);                                                       ",
                "err = sky6StarChart.LASTCOMERROR;                                              ",
                "if (err != 0){                                                                 ",
                "        Out = Target + ' not found.';                                          ",
                "}else{                                                                         ",
                "        sky6ObjectInformation.Property(54);                                    ",
                "        var targetRA = sky6ObjectInformation.ObjInfoPropOut;                   ",
                "        sky6ObjectInformation.Property(55);                                    ",
                "        var targetDec = sky6ObjectInformation.ObjInfoPropOut;                  ",
                "        sky6ObjectInformation.Property(77);                                    ",
                "        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;            ",
                "        sky6ObjectInformation.Property(78);                                    ",
                "        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;           ",
                "        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj);                  ",
                "        var slewComplete = sky6Web.IsSlewComplete;                             ",
                "        while(slewComplete != 1){                                              ",
                "                slewComplete = sky6Web.IsSlewComplete;                         ",
                "        }                                                                      ",
                "        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);   ",
                "}                                                                              ",
                "Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';               ",
                "/* Socket End Packet */                                                        "
        };

        char* set_Venus_data[] =
        {
                "/* Java Script */                                                              ",
                "/* Socket Start Packet */                                                      ",
                "var Out;                                                                       ",
                "var err;                                                                       ",
                "var obj='Venus';                                                               ",
                "sky6StarChart.LASTCOMERROR = 0;                                                ",
                "sky6StarChart.Find(obj);                                                       ",
                "err = sky6StarChart.LASTCOMERROR;                                              ",
                "if (err != 0){                                                                 ",
                "        Out = Target + ' not found.';                                          ",
                "}else{                                                                         ",
                "        sky6ObjectInformation.Property(54);                                    ",
                "        var targetRA = sky6ObjectInformation.ObjInfoPropOut;                   ",
                "        sky6ObjectInformation.Property(55);                                    ",
                "        var targetDec = sky6ObjectInformation.ObjInfoPropOut;                  ",
                "        sky6ObjectInformation.Property(77);                                    ",
                "        var tracking_rateRA = sky6ObjectInformation.ObjInfoPropOut;            ",
                "        sky6ObjectInformation.Property(78);                                    ",
                "        var tracking_rateDec = sky6ObjectInformation.ObjInfoPropOut;           ",
                "        sky6RASCOMTele.SlewToRaDec(targetRA, targetDec, obj);                  ",
                "        var slewComplete = sky6Web.IsSlewComplete;                             ",
                "        while(slewComplete != 1){                                              ",
                "                slewComplete = sky6Web.IsSlewComplete;                         ",
                "        }                                                                      ",
                "        sky6RASCOMTele.SetTracking(1, 0, tracking_rateRA, tracking_rateDec);   ",
                "}                                                                              ",
                "Out = 'Slew to ' + obj + ' completed. Tracking on RaDec rates.';               ",
                "/* Socket End Packet */                                                        "
        };
	//--------------------------------------
	//



	//      S H A R E D   M E M O R Y 
	// ------------------------------------
/*
	fd_shmem = shm_open(BackingFile,	// name from smem.h
        	O_RDWR | O_CREAT,         	// read/write, create if needed
     		AccessPerms);             	// access permissions (0644)

  	if (fd_shmem < 0) report_and_exit("Can't open shared mem segment...");

  	ftruncate(fd_shmem, ByteSize);          // get the bytes

  	rbpos_base_ptr = mmap(NULL,             // let system pick where to put segment
        	ByteSize,                       // how many bytes
                PROT_READ | PROT_WRITE,         // access protections
                MAP_SHARED,                     // mapping visible to other processes
                fd_shmem,                       // file descriptor
                0);                             // offset: start at 1st byte

	if ( (void *) -1  == rbpos_base_ptr)
    	report_and_exit("Can't get segment for shared memory...");
  	else
    	rbpos_ptr = rbpos_base_ptr;

  	//  Semaphore code to lock the shared mem
  	semptr = sem_open(SemaphoreName,      	// name
        	O_CREAT,                        // create the semaphore
                AccessPerms,                    // protection perms
                0);                             // initial value

  	if (semptr == (void*) -1) report_and_exit("sem_open");

	// F I L E  O P E N  

	if ( (fd_data = open(filename, O_RDWR | O_CREAT | O_APPEND, AccessPerms)) == -1)
	{
		fprintf(stderr, "Cannot open getposition data file. Try again later.\n");
		exit(1);
	}
	//---------------------------------------------------------------
*/




	// TCP Communication
	// -----------------


	//CREATING THE SOCKET
      	sock = socket(AF_INET , SOCK_STREAM , 0);
      	if (sock == -1)
        {
        	//printf("Could not create socket.\n");
		log_error("Could not create socket TCP");
        }
	log_info("Socket TCP created.");

      	server.sin_addr.s_addr = inet_addr(IP_SERVER);
      	server.sin_family = AF_INET;
      	server.sin_port = htons(TCP_PORT);

      	//CONNECT TO REMOTE SERVER
      	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
        {
        	//printf("Could not connect to remote socket server.\n");
          	log_error("Could not connect to remote socket server (%s:%d).", TCP_PORT, IP_SERVER);
		return 1;
        }
	log_info("Connection stablished with the server.");

	//SENDING DATA TO SERVER BASED ON ARGUMENTS
	if (strcmp(argv[1],"Sun") == 0)
	{
		log_info("Tracking to %s selected.", argv[1]);
		/************************************************************/
      		//SEND DATA
      		for(i = 0; i<command_nlines ; i++)
        	{
			if( send(sock , set_Sun_data[i] , strlen(set_Sun_data[i]) , 0) < 0)
            		{
              			//printf("Could not send data to remote socket server.\n");
				log_error("Could not send data to remote socket server.");
              			return -1;
            		}
        		usleep(TX_DELAY);
        	}
		log_info("Data instructions sent to remote server.");

	}else if (strcmp(argv[1],"Moon") == 0)
	{
		log_info("Tracking to %s selected.", argv[1]);
                /************************************************************/
                //SEND DATA
                for(i = 0; i<command_nlines ; i++)
                        {
                                if( send(sock , set_Moon_data[i] , strlen(set_Moon_data[i]) , 0) < 0)
                                {
                                        //printf("Could not send data to remote socket server.\n");
                                        log_error("Could not send data to remote socket server.");
					return -1;
                                }
                        usleep(TX_DELAY);
                }
		log_info("Data instructions sent to remote server.");

	}else if (strcmp(argv[1],"Jupiter") == 0)
	{
		log_info("Tracking to %s selected.", argv[1]);
                /************************************************************/
                //SEND DATA
                for(i = 0; i<command_nlines ; i++)
                        {
                                if( send(sock , set_Jupiter_data[i] , strlen(set_Jupiter_data[i]) , 0) < 0)
                                {
                                        //printf("Could not send data to remote socket server.\n");
                                        log_error("Could not send data to remote socket server.");
					return -1;
                                }
                        usleep(TX_DELAY);
                }
		log_info("Data instructions sent to remote server.");

        }else if (strcmp(argv[1],"Venus") == 0)
        {
		log_info("Tracking to %s selected.", argv[1]);
                /************************************************************/
                //SEND DATA
                for(i = 0; i<command_nlines ; i++)
                        {
                                if( send(sock , set_Venus_data[i] , strlen(set_Venus_data[i]) , 0) < 0)
                                {
                                        //printf("Could not send data to remote socket server.\n");
					log_error("Could not send data to remote socket server.");
                                        return -1;
                                }
                        usleep(TX_DELAY);
                }
		log_info("Data instructions sent to remote server.");
	}else{
                printf("\n\tUsage: pTrack <object> \n");
                printf("\t\tObject: Sun, Moon, Jupiter or Venus.\n\n");
		log_warn("Bad parameter when enter pTrack instructions.");
		exit(0);
	}


      	//RECEIVE A REPLY FROM THE SERVER
      	if( recv(sock , server_reply , RCV_BUFFER_SIZE , 0) < 0)
        {
        	//printf("Could not receive data from socket server.\n");
		log_error("Could not receive data from socket server.");
          	return -1;
        }


      	//=================================================================
      	close(sock);
      	//=================================================================


  	printf("\nServer Reply: %s\n\n", server_reply);

	log_info("Server Reply: %s", server_reply);

	log_info("Socket TCP closed.");


	fclose(fp);

  	return 0;
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