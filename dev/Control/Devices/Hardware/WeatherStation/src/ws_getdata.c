/*
        ================================================================================
                             Universidade Presbiteriana Mackenzie
                 Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        ================================================================================

        Weather Station Get Data version 0.2
*/
#define VERSION "0.2"
/*
        --------------------------------------------------------------------------------
        Este programa abre uma conexao serial com a estacao meteorologica Vaisala 
	WXT530 via /dev/ttyUSB0, envia a instrução de coleta da leitura dos sensores de 
	temperatura, umidade e pressao "0R2!", separa os campos e armazena em arquivo
	binario contendo um timestamp HUSEC. Podendo ser executado no modo verbose ou
	como daemon, permitindo ser inicializado durante o boot via systemctl. 
        --------------------------------------------------------------------------------

        Autor: Tiago Giorgetti                        tgiorgetti@gmail.com

        --------------------------------------------------------------------------------

        Histórico:
        ________________________________________________________________________________
         Versão |  Data         |       Atualização
        --------------------------------------------------------------------------------
          0.1   |  07-12-2021   | Primeira versão. Apenas o codigo basico para conexao e
		|		| coleta com exibicao na tela.
        --------------------------------------------------------------------------------
          0.2   |  		| Implementacao de opcoes com getopt().
                |               | 
        ________|_______________|_______________________________________________________

        Implementação do kbhit() na referencia baixo:
        https://www.raspberrypi.org/forums/viewtopic.php?t=188067 - acesso em 04-10-2019.

*/



/*
 *	Compilar com a opcao DISPLAY_STRING para nao converter para hexadecimal
 *	
 *	gcc -DDISPLAY_STRING ws_getdata.c -o ws_getdata
 *
 *
 *	Ref.: https://stackoverflow.com/questions/6947413/how-to-open-read-and-write-from-serial-port-in-c
 *	Ref.: https://www.cmrr.umn.edu/~strupp/serial.html
 *
*/




#define TERMINAL    "/dev/ttyUSB0"


#include <termio.h>
#include <stdint.h>
#include <inttypes.h>

#include <sys/resource.h>       //For DAEMON


#include <signal.h>     //For Guigue's timing procedures 
#include <sys/time.h>   //For Guigue's timing procedures

#include <errno.h>      //For Guigue's rotative file save
#include <time.h>       //For Guigue's rotative file save



#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>

#include "ws-getdata.h"
#include "husec.h"      //Time Hundred of Micro Seconds (Husec) 

        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
        *****************************************************************/

char * strmycpy(char *dest, const char *src, int nbegin, int nend);
static void ExitOnDemand ( int sigcode, siginfo_t * siginfo, void *context );


bool kbhit(void);
int set_interface_attribs(int fd, int speed);
void set_mincount(int fd, int mcount);

//Guigue's Rotative File Save Functions
void save_data( int, ws_data_type *, size_t);
void close_file ( void )        ;
void open_file ( struct tm * )  ;

//Guigue's Timming Procedures - new funcion
//Context modification from version 0.6.1
void ReadWS( int ) 	        ;





        // Tratamento de parametros via getopt()
        // -------------------------------------

//Opt Flag Variables
static int version_flag         ;
int verbose_flag = 0            ;
int interval_flag = 0		;
int help_flag = 0               ;
int daemon_flag = 0             ;
int stop_flag = 0               ;

//Opt Arguments
char *argument = NULL           ;
char *interval_value  = "5"     ;       // Default Weather Station read time interval = 5 seconds

int optflag_ctr = 0             ;




	// Others GLOBAL VARIABLES
        // -----------------------



// ----Printing information on screen
int k = 0					;       //For screen print
const char spin[4]={'|', '/', '-', '\\'}	;       //For screen print      



ws_data_type * ws_data				;       //Ponteiros do Ring Buffer


int fd, fd_data, fd_data_w			;       //Used to Open and Write a Binary Data File
char ws_file_name[120]				;       //GLOBAL FOR GUIGUE's SAVING ROTATIVE FILE


static char * SDTDPID = "/opt/HAX/dev/Control/Devices/Hardware/Mount/log/getPos_daemon.pid"	;       //From config file - DAEMON PID
static char * DATA_DIR = "/opt/HAX/dev/Control/Devices/Hardware/Mount/src/" 			;       //From config file - Specific

char * p					;       //Buffer String Tok strtok()
char psep;
const char sep[2] = ","				;       //Buffer String Tok strtok()





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
                        {"version", no_argument,        &version_flag,  1},
                        // These options don't set a flag. We distinguish them by their indices
                        {"verbose",	no_argument,         0,             'v'},
			{"interval",	required_argument,   0,		    'i'},
                        {"help",	no_argument,         0,             'h'},
                        {"daemon",	no_argument,         0,             'd'},
                        {"stop",	no_argument,         0,             's'},
                        {0, 0, 0, 0}
                };

                // getopt_long stores the option index here.
                int option_index = 0;

                c = getopt_long (argc, argv, "vi:hds", long_options, &option_index);
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

			case 'i':
				interval_flag = 1;
				optflag_ctr++;
				if (optarg) interval_value = optarg;
				//printf("interval_flag: %d\n",interval_flag);
				break;

                        case 'h':
                                help_flag = 1;
                                optflag_ctr++;
                                //printf("help_flag: %d\n",help_flag);
                                break;
                        case 'd':
                                daemon_flag = 1;
                                optflag_ctr++;
                                //printf("daemon_flag: %d\n",daemon_flag);
                                break;
                        case 's':
                                stop_flag = 1;
                                optflag_ctr++;
                                //printf("stop_flag: %d\n",stop_flag);
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
                while (optind < argc)
                {
                        if (verbose_flag) printf("%s \n", argv[optind]);
                        optind++;
                }
                if (diff >= 1)
                {
                        printf("\n");
                        printf("ws_getdata: I don't have any valid arguments, only options. Try -h or --help.\n");
                        exit(1);
                }
                if (verbose_flag) printf("\rws_getdata: I don't care about this = %s                 \n",argument);
        }

        if (argc < 2)
        {
                printf("ws_getdata: There is no options, please try -h or --help.\n\n");
                exit(1);
        }

        // Options Errors Verification
        if (argument && memcmp(argument,"-",1) == 0)
        {
                puts("ws_getdata: Object Sintax Error! Try -h or --help.");
                exit(1);
        }

        if (help_flag == 1 && (argc > 2 || optflag_ctr > 1))
        {
                puts("ws_getdata: If you need help, don't input more options or parameters! Try -h or --help.");
                exit(1);
        }

        if (version_flag == 1 && argc > 2)
        {
                puts("ws_getdata: For version information must be used without another option! Try -h or --help.");
                exit(1);
        }

	if (stop_flag == 1 && argc > 2)
	{
   		puts("ws_getdata: To stop 'ws-getdata' it  must be used without another option! Try -h or --help.");
                exit(1);
        }

	if (daemon_flag == 1 && verbose_flag == 1)
	{
	        puts("ws_getdata: Is not possible to use the verbose option while daemon option is set! Try -h or --help.");
                exit(1);
        }


	// Print help text in the screen
        if (help_flag)  print_usage();


        //=========================
        // Print version code
        if (version_flag)
        {
                printf("-----------------------------------------\n");
                printf("Weather Station Get Data - version %s\n", VERSION);
                printf("-----------------------------------------\n");
                exit(0);
        }


	//=========================
	// Verbose Initial Routines

	if(verbose_flag == 1) system("clear"); // Clean Screen
        if(verbose_flag == 1) printf("----------------------\n");
        if(verbose_flag == 1) printf("Verbose option is set!\n");
        if(verbose_flag == 1) printf("----------------------\n");
        if(verbose_flag == 1) printf("optind = %d\n",optind);
        if(verbose_flag == 1) printf("argc = %d\n",argc);
        if(verbose_flag == 1) printf("optflag_ctr = %d\n",optflag_ctr);
        if (interval_flag == 1)
                if(verbose_flag == 1) printf("interval_value = %s [seconds]\n\n",interval_value);
        //printf("\nRun the ws_getdata instructions!\n\n");






	// DECLARACAO VARIAVEIS DA FUNCAO PRINCIPAL
        // ----------------------------------------

        // DAEMON Variables
        struct rlimit resourceLimit = {0};
        int status = -1;
        int fileDesc = -1;
        int id_rsc;
        struct sigaction catchSignal;
        struct stat pid_file_stat;
        FILE * sdtdpid;




        // CONVERTION OF STRING TO THEIR RELATIVE DECIMALS
        int     interval_dvalue               ;
	interval_dvalue = atoi(interval_value);
        if (verbose_flag == 1) printf("interval_dvalue = %d [seconds]\n", interval_dvalue);





	// Connection to the Weather Station Port
	// --------------------------------------

        fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);

        if (fd < 0)
        {
                printf("Error opening %s: %s\n", portname, strerror(errno));
                return -1;
        }

        /*baudrate 19200, 8 bits, no parity, 1 stop bit */
        set_interface_attribs(fd, B19200);
        set_mincount(fd, 0);                /* set to pure timed read */

        /* simple output */
        wlen = write(fd, xstr, xlen);
        if (wlen != xlen)
        {
                printf("Error from write: %d, %d\n", wlen, errno);
        }
        tcdrain(fd);    /* delay for output */


        /* simple noncanonical input */
        unsigned char buf[80];
        int rdlen;
        rdlen = read(fd, buf, sizeof(buf) - 1);



	
	// Preset for Deamon Routine
	// -------------------------

        if (stop_flag)
        {
                char dt_pid[10]     ;
                FILE * sdtdpidfile  ;

                sdtdpidfile = fopen(SDTDPID,"r");
                if (sdtdpidfile != NULL)
                {
                        fscanf(sdtdpidfile,"%s",dt_pid);
                        fclose(sdtdpidfile);
                        printf("\n\n Killing precal processes = %s\n\n",dt_pid);
                        execl("/bin/kill","kill","-s","TERM",dt_pid,(char *) NULL);
                } else {
                        printf("\n\n I don't find %s\n Aborting...\n\n",SDTDPID);
                }
        }


        // DAEMON DEFINITION
        // -----------------
        // This part starts the operations to convert the 
        // program to a Unix Daemon.
        //
        // The following procedure was taken from the article:
        // "Linux Network Programming, Part 2: Creating Daemon Processes"
        // By Ivan Grifith and John Nelson
        // From: Linux Journal, 47, March 1998
        // --------------------------------------------------------------

        if (daemon_flag)
        {
                if (0 == stat(SDTDPID, &pid_file_stat))
                {
                        printf("\n\n There is a session of %s running. Exiting... \n\n",argv[0]);
                        //log_info("There is a session of %s running. Exiting...",argv[0]);
                        exit(1);
                }

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

                if (-1 == status)
                {
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
                        if (sdtdpid != NULL)
                        {
                                fprintf(sdtdpid,"%d\n",status) ;
                                fclose(sdtdpid);
                        }
                        exit(0);
                }

                //log_info("getPos daemon was started.. running in background.");

                // Now we are in a new session and process group
                // than process that started the daemon. We also
                // have no controlling terminal.


                chdir("/");
                umask(0);       // Umask is inherited from parents.  
                                // It's a good practice to set to 0

                fileDesc = open("/dev/null", O_RDWR); // stdin
                (void) dup(fileDesc);                 // stdout
                (void) dup(fileDesc);                 // stderr
        }
        // END OF DAEMON DEFINITON              
        // -----------------------




        /*
         *  Catch SIGINT, SIGTERM, SIGKILL
         */
        memset(&catchSignal, '\0', sizeof(catchSignal));
        catchSignal.sa_sigaction = &ExitOnDemand       ;
        catchSignal.sa_flags = SA_SIGINFO              ;
        sigaction(SIGTERM, &catchSignal, NULL)         ;
        sigaction(SIGINT, &catchSignal, NULL)          ;



        /***
        *     I N F I N I T E    L O O P
        *****************************************************************/


        //----------------------------------------------oOo--------------------------------------------------
        // Guigue's timming routine
        signal(SIGVTALRM, ReadWS);                        // function to handle the interruption
        timer.it_value.tv_sec = 0;
        timer.it_value.tv_usec = interval_dvalue*1.0E+06;            // configure the interval timer
        timer.it_interval.tv_sec = 0;
        timer.it_interval.tv_usec = interval_dvalue*1.0E+06;
        setitimer(ITIMER_VIRTUAL, &timer, NULL);             // Start a VIRTUAL itimer
        //----------------------------------------------------------------------------------------------------

        if (daemon_flag)
        {
		//log_info("Daemon started. Getting Weather Station data...");
                for(;;);
        }else{
                //log_info("Verbose execution started. Getting Weather Station data...");
                while(!kbhit());
        }

        //----------------END OF INFITITE LOOP----------------------------



       if (!daemon_flag)
        {
                //log_info("ws-getdata via screen (verbose) stoped by user.");
                //---------------------------------------------------------------
                fclose(fp);
                close(fd_data);
                return 0;
        }
}






        /***
        *     O T H E R S    F U N C T I O N S
        *****************************************************************/



static void ExitOnDemand ( int sigcode, siginfo_t * siginfo, void *context  )
{

        //log_info("ws-getdata was killed.");

        //---------------------------------------------------------------

        fclose(fp);
        close(fd_data);

        remove(SDTDPID);
        //log_info("getPos was killed. PID file was removed.");
}



void ReadWS( int sig )
{

	// Weather Station Data FEEDING

        ws_data->time_Husec = husec_time();

	
	// Conections variables to Weather Station
        char *portname = TERMINAL;
        int fd;
        int wlen;
        char *xstr = "0R2!";
        int xlen = strlen(xstr);


	if (rdlen > 0)
	{
		#ifdef DISPLAY_STRING
		buf[rdlen] = 0;
		if(verbose_flag == 1) printf("Read %d: %s \n", rdlen, buf);
		#else /* display hex */
		unsigned char   *p;
		if(verbose_flag == 1) printf("Read %d:", rdlen);
		for (p = buf; rdlen-- > 0; p++)
			printf(" 0x%x", *p);
		if(verbose_flag == 1) printf("\n");
		#endif
	} else if (rdlen < 0) {
		if(verbose_flag == 1) printf("Error from read: %d: %s\n", rdlen, strerror(errno));
	} else {  /* rdlen == 0 */
		if(verbose_flag == 1) printf("Timeout from read\n");
	}  
	if(verbose_flag == 1) printf("\rLoading... [%c]\n", spin[k]);

	fflush(stdout);
	k++;
	if (k == 4) { k = 0; };
	
	/* repeat read to get full message */
//	usleep(interval_dvalue*1.0E+06);
	
	p = strtok(buf,sep);
        if ( p != NULL)
        {
                strmycpy(psep,p,3,7);
		ws_data->temp = atof(psep);
                p = strtok(NULL,sep);
		if(verbose_flag == 1) printf("....%f....\n",psep);
        }

        if ( p != NULL)
        {
        	strmycpy(psep,p,3,7);
		ws_data->rh = atof(psep);
                p = strtok(NULL,sep);
		if(verbose_flag == 1) printf("....%f....\n",psep);
        }

        if ( p != NULL)
        {
		strmycpy(psep,p,3,7);
                ws_data->pressure = atof(psep);
                p = strtok(NULL,sep);
		if(verbose_flag == 1) printf("....%f....\n",psep);
        }
	


	
}


char * strmycpy(char *dest, const char *src, int nbegin, int nend)
{
	int i;
	
	for (i = nbegin ; i < nend && src[i] != '\0'; i++)
		dest[i] = src[i];

	return dest;
}




// KeyBoard Hit

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


// Serial Interface Attributes

int set_interface_attribs(int fd, int speed)
{
	struct termios tty;
	
	if (tcgetattr(fd, &tty) < 0)
	{
		printf("Error from tcgetattr: %s\n", strerror(errno));
		return -1;
	}

	cfsetospeed(&tty, (speed_t)speed);
	cfsetispeed(&tty, (speed_t)speed);

	tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;         /* 8-bit characters */
	tty.c_cflag &= ~PARENB;     /* no parity bit */
	tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
	tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

	/* setup for non-canonical mode */
	tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	tty.c_oflag &= ~OPOST;

	/* fetch bytes as they become available */
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 1;

	if (tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		printf("Error from tcsetattr: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

// Serial Interface auxiliar attributes

void set_mincount(int fd, int mcount)
{
	struct termios tty;

	if (tcgetattr(fd, &tty) < 0)
	{
		printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN] = mcount ? 1 : 0;
	tty.c_cc[VTIME] = 5;        /* half second timer */

	if (tcsetattr(fd, TCSANOW, &tty) < 0)
		printf("Error tcsetattr: %s\n", strerror(errno));
}



// Guigues's Rotative File Save
// ----------------------------

void save_data ( int fd_data, ws_data_type * wbpos_ptr, size_t WB_ByteSize)
{
        //int fd_data_w;        //---> Set as global variable
        static unsigned char first=1;
        static int old_mon=0, old_mday=0, old_hour=0;
        time_t time_now;
        struct tm * time_now_cal;

        time_now = time(NULL);
        time_now_cal = gmtime(&time_now);

        if (first==1)
        {
                open_file(time_now_cal);
                first = 0;
                old_mon   = time_now_cal->tm_mon       ;
                old_mday  = time_now_cal->tm_mday      ;
        } else
        {
                if ( (old_mon  != time_now_cal->tm_mon   ) ||
                     (old_mday != time_now_cal->tm_mday  ) )
                {
                        close_file();                    // close open file
                        open_file(time_now_cal);
                        old_mon   = time_now_cal->tm_mon       ;
                        old_mday  = time_now_cal->tm_mday      ;
                }
        }

        fd_data_w = write(fd_data, wbpos_ptr, WB_ByteSize);
        if (fd_data_w != WB_ByteSize)
        {
                char msg[80] ;
                sprintf( msg , "Writing ENV file failed.  Error = %d", errno );
                log_fatal("Calling process was killed. When trying to write ENV file. Error = %d", errno);
                close_file ();
                exit(1);
        }
}


// Guigue's Rotative File Open
// ---------------------------

void open_file ( struct tm * time_now_cal )
{
        char msg[120];
//      char ws_file_name[120];       //---> Set as global variable
        char ws_full_file_name[120];
        int FPERM = (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        int OPENS ;
        struct stat filestat;

        sprintf(ws_file_name, "hats-%.4d-%.2d-%.2d.env",
                        time_now_cal->tm_year+1900,
                        time_now_cal->tm_mon+1,
                        time_now_cal->tm_mday);

        sprintf(ws_full_file_name,"%s/%s",DATA_DIR,ws_file_name);

        if (-1 == stat(ws_full_file_name, &filestat))
        {
                OPENS = O_RDWR | O_CREAT | O_SYNC | O_TRUNC ;
        } else
        {
                OPENS = O_APPEND | O_RDWR | O_SYNC ;
        }

        fd_data = open(ws_full_file_name, OPENS , FPERM ) ;

        if (fd_data == -1)
        {
                char msg[160] ;
                sprintf( msg ,"Open ENV file %s failed. Error = %s", ws_full_file_name,strerror(errno));
                log_fatal("Calling process was killed. When trying to open ENV file: %s Error = %s", ws_full_file_name,strerror(errno));
                exit(1);
        } else
        {
                sprintf( msg ,"Opened ENV file  %s", ws_file_name);
        //      log_info("Opened ENV file: %s", ws_file_name);
        }
}



// Guigue's Rotative File Close
// ----------------------------

void close_file (void)
{
        char msg[120];
        close(fd_data);
        sprintf( msg, "Closed ENV file %s", ws_file_name) ;
        //log_info("Closed ENV file: %s", ws_file_name);
}


