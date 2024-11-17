#ifndef WSGETDATA
#define WSGETDATA

//Parametros de conexao
//#define IP_SERVER "10.0.92.12"
//#define TCP_PORT 3040
//#define RCV_BUFFER_SIZE 2000
//#define TX_DELAY 0		//Microseconds


//Parametros para Husec_time()
//#define SEC2HUSEC     10000L
//#define HUSEC2NSEC   100000L
//#define ONEDAY        86400L
//#define HUSECS2HRS 36000000L
//#define MIN2HUSEC    600000L
//#define MIN              60L


//Parametros para Shared Memory
//#define BackingFile "HatsRingBuffer"
//#define SemaphoreName "HatsSemaphore"
//#define AccessPerms 0664


//Parametros do Ring Buffer
//#define RINGSIZE 10

//#define DATAFILENAME  "getposition_file.bin"



typedef struct
{
  unsigned long long time_Husec 	;	//Time: Husec
  //--------------------------------------------------------
  float temp				;	//Weather Station: Temperature in Celsius
  float rh				;	//Weather Station: Relative Humidity
  float pressure			;	//Weather Station: Pressure in mmHg
  //---------------------------------------------------------
} ws_data_type ;



//----------Begin--Of--Help--------------------------------------------

int print_usage()
{
	printf("\n\n  ws-getdata - Weather Station Get Data.\n\n\n");
	
	
	printf("SYNOPSIS\n\n");
	
	printf(" ws-getdata [OPTIONS]\n\n");
	
	printf("DESCRIPTION\n\n");
	
	printf(" This program performs, in a infinite loop, data aquisition from    \n");
	printf(" Weather Station Vaisala WXT534 serial number S3420785 to gather    \n");
	printf(" Temperature [Celsius], Relative Humidity [RH] and Pressure [mmHg]. \n");
	printf(" __________________________________________________________________ \n");
	printf(" Husec| Temperature | Relative Humidity | Pressure                  \n");
	printf("   |	    |		   |		     |			    \n");
	printf("   |	  float          float	           float                    \n");				   
	printf("   |		   	   					    \n");
	printf("  unsigned long long						    \n");
	printf(" __________________________________________________________________ \n");
	printf(" Was built to working in background, but for tests, is possible     \n");
	printf(" some options as described below.  				  \n\n");
	
	printf("OPTIONS\n\n");
	
	printf(" -h, --help       Print this help and exit.\n");
	printf(" -v, --verbose    Verbose option. Basic output informations.\n");
	printf(" -i, --interval   This option sets the weather station read time interval.\n");
	printf(" -d, --daemon	  Run ws-getdata as a daemon executing in background.\n");
	printf("		  This option have safe instruction using SIGTERM, by\n");
	printf("		  closing all descriptors and memories before be killed.\n");
	printf("		  This option must be used without another option.\n");
	printf(" -s, --stop	  This option read the PID file and kill the process\n");
	printf("		  by sending a SIGTERM signal.\n");
	printf(" --debug          Full output information from ws-getdata and log.\n");
	printf(" --version        Show ws-getdata's version and exit.\n\n");
	
	printf("AUTHOR\n\n");
	
	printf(" Written by Tiago Giorgetti and Guillermo de Castro\n\n");
	
	printf("REPORTING BUGS\n\n");
	
	printf(" Please send an email to tgiorgetti@gmail.com or guigue@craam.mackenzie.br\n\n");
	
	printf("COPYRIGHT\n\n");
	
	printf(" Copyright © 2021 Free Software Foundation, Inc. License GPLv3+: GNU GPL version 3  or later\n");
	printf(" <https://gnu.org/licenses/gpl.html>.\n");
	printf(" This  is free software: you are free to change and redistribute it.\n");
	printf(" There is NO WARRANTY, to the extent permitted by law.\n\n");
        
	printf("SEE ALSO\n\n");
	
	printf("Full documentation at: <http://github.com>\n\n");
	
	exit(0);
}




















#endif
