/*

  Author: Guigue - September 1999, CASLEO
                   September 11 2021, Home

*/


#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "data_transfer.h"

#define ENOPIDFILE 1
#define SUCCESS 0
#define VERSION 20211215T1200BRT

int main(int argc, char ** argv ){

  char dt_pid[10]     ;
  FILE * sdtdpidfile  ;
  static int verbose=0, help=0 ; 
  int option, option_index;
  
  static struct option long_options[] =
        {
	 {"verbose",  no_argument, &verbose, 1},
	 {"help", no_argument, &help, 1},	 
	 {0, 0, 0, 0}
        };  
  

  while (1)
    {
      option = getopt_long (argc, argv, "", long_options, &option_index);

      /* Detect the end of the options. */
      if (option == -1)
	break;
      
      switch (option)
	{
	case 0:
	  /* If this option set a flag, do nothing else now. */
	  if (long_options[option_index].flag != 0)
	    break;
	  printf ("option %s", long_options[option_index].name);
	  if (optarg)
	    printf (" with arg %s", optarg);
	  printf ("\n");
	  break;
	  
        case '?':
          /* getopt_long already printed an error message. */
          break;
	  
        default:
          abort ();
	}
    }
  
  sdtdpidfile = fopen(SDTDPID,"r");
  
  if (sdtdpidfile != NULL) {
    
    fscanf(sdtdpidfile,"%s",dt_pid);
    fclose(sdtdpidfile);
    
    execl("/bin/kill","kill","-s","TERM",dt_pid,(char *) NULL);
    if (verbose) printf("\n\n Killing data acquisition processes = %s\n\n",dt_pid);
    exit(SUCCESS);
      
  } else {
    if (verbose) printf("\n\n I don't find %s\n Aborting...\n\n",SDTDPID);
    exit(ENOPIDFILE);
  }
  
}
