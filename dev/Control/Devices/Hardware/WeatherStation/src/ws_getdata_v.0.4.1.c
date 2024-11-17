/*
        ================================================================================
                             Universidade Presbiteriana Mackenzie
                 Centro de Rádio Astronomia e Astrofísica Mackenzie - CRAAM
        ================================================================================

        Weather Station Get Data version 0.4.1
*/
#define VERSION "0.4.1"
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
          0.1   |  07-12-2021   | Primeira versão. Apenas o código básico para conexão e
		|		| coleta com exibição na tela.
        --------------------------------------------------------------------------------
          0.2   |  01-02-2023	| Implementação de opções com getopt().
        --------------------------------------------------------------------------------
       	  0.2.1 |  02-09-2024	| Correção da rotina de coleta. Valores lidos não 	
		|		| estavam sendo atualizados da estação.
	--------------------------------------------------------------------------------
	  0.3	|  02-09-2024	| Implementação de rotina de armazenagem em arquivo 
	  	|		| binário com rotatividade diária.
	--------------------------------------------------------------------------------
	  0.4	|  08-09-2024	| Substituicao da rotina utilizando função strtok() e
	  	|		| criar rotina para geracao de dados CSV.
	--------------------------------------------------------------------------------
	  0.4.1 |  10-09-2024	| Separacao de rotinas "verbose" de rotinas para serem 
  		|		| colocadas em backgrond. Aniversario da Nicole (16 anos).	  
        ________|_______________|_______________________________________________________

        Implementação do kbhit() na referencia baixo:
        https://www.raspberrypi.org/forums/viewtopic.php?t=188067 - acesso em 04-10-2019.

*/



/*
 *	Compilar com a opcao DISPLAY_STRING para nao converter para hexadecimal
 *	
 *	gcc -DDISPLAY_STRING -Wall -I../inc ws_getdata_v.0.3.c -o ws_getdata_v.0.3
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




#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <getopt.h>
#include <time.h>

#include "ws-getdata.h"

#include <errno.h>      //For Guigue's rotative file save
#include <time.h>       //For Guigue's rotative file save

#include "husec.h"      //Time Hundred of Micro Seconds (Husec)


        /***
        *     P R E L I M I N A R Y    F U N C T I O N S
        *****************************************************************/




//-----Serial Comunication for WeatherStation
bool kbhit(void);
int set_interface_attribs(int fd, int speed);
void set_mincount(int fd, int mcount);

//-----Guigue's Rotative File Save Functions
void save_data(char *);
void close_file ( void )        ;
void open_file ( struct tm * )  ;




        /***
        *     G L O B A L   V A R I A B L E S
        *****************************************************************/


//-----Tratamento de parametros via getopt()


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


//Printing information on screen
int k = 0                          		;       //For screen print
const char spin[4]={'|', '/', '-', '\\'}        ;       //For screen print      


//Rotative storage variables
time_t rawtime			;  
struct tm * ti			;
char isotime[20]		;
char ws_data[200]		;

FILE * fd_data			;
char ws_file_name[120]		;       //GLOBAL FOR GUIGUE's SAVING ROTATIVE FILE

//static char * SDTDPID = "/opt/HAX/dev/Control/Devices/Hardware/Mount/log/getPos_daemon.pid"     ;       //From config file - DAEMON PID
static char * DATA_DIR = "/opt/HAX/dev/Control/Devices/Hardware/WeatherStation/data/"           ;       //From config file - Specific







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

	//        if (argc < 2)
	// {
	//         printf("ws_getdata: There is no options, please try -h or --help.\n\n");
	//         exit(1);
	//  }

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


        // CONVERTION OF STRING TO THEIR RELATIVE DECIMALS
        // -----------------------------------------------
        int interval_dvalue;
	interval_dvalue = atoi(interval_value);
        if (verbose_flag == 1) printf("interval_dvalue = %d [seconds]\n", interval_dvalue);

	

	// Conections variables

	char *portname = TERMINAL;
	int fd;
	int wlen;
	char *xstr = "0R2!";
	int xlen = strlen(xstr);

	fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);

	if (fd < 0)
	{
	        if(verbose_flag) printf("Error opening %s: %s\n", portname, strerror(errno));
		return -1;
	}
	
	/*baudrate 19200, 8 bits, no parity, 1 stop bit */
	set_interface_attribs(fd, B19200);
	set_mincount(fd, 0);                /* set to pure timed read */
		
	do
	{

		/* simple output */
		wlen = write(fd, xstr, xlen);
		if (wlen != xlen)
		{
			if(verbose_flag) printf("Error from write: %d, %d\n", wlen, errno);
		}
		tcdrain(fd);    /* delay for output */
		
		/* simple noncanonical input */
		unsigned char buf[80];
		int rdlen;
		rdlen = read(fd, buf, sizeof(buf) - 1);
		//if (rdlen > 0)  buf[rdlen] = 0;
		
		if(verbose_flag)
		{
			system("clear"); // Clean Screen
			printf("----------------------\n");
			printf("Verbose option is set!\n");
			printf("----------------------\n");
			printf("optind = %d\n",optind);
			printf("argc = %d\n",argc);
			printf("optflag_ctr = %d\n",optflag_ctr);
		  
			if (interval_flag == 1)
		      	{
				if (interval_dvalue == 1)
				{
			        	printf("interval_value = %s second\n\n",interval_value);
			  	} else
					printf("interval_value = %s seconds\n\n",interval_value);
			} else
				printf("interval_value = 5 seconds (Default)\n\n"); 	
		}
		if(verbose_flag) printf("\nRun the ws_getdata instructions!\n\n");

               	
		if (rdlen < 0)
		{		
			if(verbose_flag) printf("Error from read: %d: %s\n", rdlen, strerror(errno));
		}else if (rdlen > 0)
		{
			#ifdef DISPLAY_STRING
                       	buf[rdlen] = 0;
                        if(verbose_flag) printf("Read %d: %s \n", rdlen, buf);
                        #else /* display hex */
                        unsigned char * p;
                        if(verbose_flag) printf("Read %d:", rdlen);
                        for (p = buf; rdlen-- > 0; p++)
                        	if(verbose_flag) printf(" 0x%x", *p);
                        if(verbose_flag) printf("\n");
                        #endif
		}else {  /* rdlen == 0 */
			if(verbose_flag) printf("Timeout from read\n");
		}
		if(verbose_flag)
		{
			printf("\rLoading... [%c]\n\n", spin[k]);
			fflush(stdout);
			k++;
			if (k == 4) { k = 0; };
		}
			
		time (&rawtime);
		ti = localtime(&rawtime);
		sprintf(ws_data,"%04d-%02d-%02dT%02d:%02d:%02d,%s",ti->tm_year+1900,
			ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec,buf);

		// Weather Station Data FEEDING
 		save_data(ws_data);

		/* repeat read to get full message */
		usleep(interval_dvalue*1.0E+06);
	}while(!kbhit());
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

void save_data (char * ws_data)
{
        static unsigned char first=1;
        static int old_mon=0, old_mday=0;
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

        fprintf(fd_data, "%s",ws_data);
	fflush(fd_data);
}


// Guigue's Rotative File Open
// ---------------------------

void open_file ( struct tm * time_now_cal )
{
        char msg[200];
        char ws_full_file_name[150];

        sprintf(ws_file_name, "hats-%.4d-%.2d-%.2d.ws",
                        time_now_cal->tm_year+1900,
                        time_now_cal->tm_mon+1,
                        time_now_cal->tm_mday);

        sprintf(ws_full_file_name,"%s/%s",DATA_DIR,ws_file_name);

        fd_data = fopen(ws_full_file_name, "a") ;

        if (fd_data == NULL)
        {
                sprintf( msg ,"Open WeatherStation file %s failed. Error = %s", ws_full_file_name,strerror(errno));
                exit(1);
        } else
        {
                sprintf( msg ,"Opened WeatherStation file  %s", ws_file_name);
        }
}



// Guigue's Rotative File Close
// ----------------------------

void close_file (void)
{
        char msg[200];
        fclose(fd_data);
        sprintf( msg, "Closed WeatherStation file %s", ws_file_name);
}

