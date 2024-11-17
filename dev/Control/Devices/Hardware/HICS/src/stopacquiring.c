/*

  restartdts.c

  This program was written to fix a problem we have with the
  ON-Line display.  When "sst_ui" (the IDL program) starts
  it reads the whole file and shows in the graphical window.

  This is annoyant.

  One way to fix outside "sst_ui" (the normal way to do it,
  would be to fix it "inside" sst_ui, but presently we need 
  more programmers ) this problem, is to restart the server 
  which runs on Twain.

  This is exactly what this program does.  First it killis the
  server, then it restarts it.

  The server (sdtd) is a daemon run by root.  This is necessary
  as it opens an Internet communication.  But in Linux is 
  forbidden to setuid a script file.

  Then the only solution was to create a C program to fork(), and
  execl().

  This program can be used in a script file to start the sst_ui.
  For instance:

          #!/bin/sh
	  /data/twain/bin/restartdts
	  idl /usr/local/sst/idl/sst_ui

  Author: Guigue - September 1999, CASLEO

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
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>


main(){

  int cstatus,status = -1;

  if (sdtdpidfile != NULL) {
    fscanf(sdtdpidfile,"%s",&sdtdpid);
    printf("\n\n Killing data transfer process = %s\n\n",sdtdpid);
    execl("/bin/kill","kill","-KILL",sdtdpid,NULL);
    
    fscanf(sdtdpidfile,"%s",&sdtdpid);
    printf("\n\n Killing precal process = %s\n\n",sdtdpid);
    execl("/bin/kill","kill","-KILL",sdtdpid,NULL);
        
  } else printf("\n\n I don't find %s\n Aborting...\n\n",SDTDPID);
  
}
