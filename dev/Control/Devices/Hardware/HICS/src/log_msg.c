/*************************************************************

                            log_msg.c

 This is the ``new'' log_msg program.  It uses syslog() to
 log the messages in different files.  Files are defined in
 files.h

	   /data/log/shmem.msg      for shared memory
	   /data/log/mqueue.ms      for mqueue
	   /data/log/tcpip.msg      for TCP/IP communication between
                                    node2 and display computer (twain)
	   /data/log/files.msg      for any file operation
	   /data/log/acquire.m      for acquire2 (and related issues) 
	   /data/log/control.m      for control2 (and related issues)
	   /data/log/ephem.msg      for ephemeris 
	   /data/log/other.msg      miscelaneous

  Each ``main'' program should open the register itself with the
  syslogd daemon trough openlog():

            #include <syslog.h>
	    #include include/files.h

	    char *  prname = "program_name";
	    extern void log_msg( unsigned short int, char *, unsigned short int);

	    main(){
  
            openlog("programname", LOG_NDELAY, LOG_OTHER) ;

            :
            : body of the program
            :

            closelog();   [before exiting the program]

	    }

  In subroutines not included in the same module as the main should be included

            #include <syslog.h>
	    #include include/files.h
	    extern void log_msg( unsigned short int, char *, unsigned short int);

	    function (....) {
            :
	    : body of the program
	    :
	    }

  For programs running on node2 is also needed to add 

            #include <process.h>

	    putenv("SYSLOG=1");

  before the openlog().

  The use of log_msg() remains as in the first version:

           void log_msg (unsigned short int, char *, unsigned short int)
           log_msg(LOG_FILE , msg , prio )

   where:

           LOG_FILE is one of the following

		   LOG_SHMEM
		   LOG_MQUEUE
		   LOG_TCPIP
		   LOG_FILES
		   LOG_ACQ
		   LOG_EPHEM
		   LOG_OTHER

	   msg is any string message we want to keep, but is good
	   to add the name of the calling subroutine. 

	   prio is one of the following:

                   EMERGENCY      System unusable. The message will be 
                                  send to all the consoles.

                   ALERT          Message will be showed in one special
                                  terminal (still to be done).

                   NOTICE         Message will be saved.

   EMERGENCY, maps to LOG_ALERT in syslogd, ALERT to LOG_WARNING and 
   NOTICE to LOG_NOTICE.  I have to do it in this way because there
   is some "bug" in QNX which makes ignore the LOG_EMERG (highest)
   priority level.  See files.h 

   The /etc/syslog.conf for QNX should be:


     *.*		/tmp/syslog
     news.*	/data/log/other.msg
     user.*	/data/log/ephem.msg
     local0.*	        /data/log/shmem.msg
     local1.*	        /data/log/mqueue.msg
     local2.*	        /data/log/tcpip.msg
     local3.*	        /data/log/files.msg
     local4.*	        /data/log/acquire.msg
     local5.*	        /data/log/control.msg

   There is a copy of this file in /usr/local/sst/etc/qnx and other in  
   /usr/local/sst/etc/linux 

   The syslogd daemon should be restarted after each modification of this
   file (kill -HUP ...; is enough).  The second line makes every EMERGENCY
   (LOG_ALERT) message to be displayed on every terminal (even pseudo ones)
   with a beep.  

   In order to print the messages in the "SST Console" of Node2 is
   necessary to open a pterm:

     pterm -d /dev/ptypf -g10x80 -t "SST Console"


 *************************************************************

 Guigue, IAP, May 1998

 *************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <termios.h>
#include "data_transfer.h"

/* File names deffinitions for the ``new'' log_msg

	IMPORTANT

	If you change something here, you HAVE to change also
	the /etc/syslog.conf (a copy of it is in /usr/local/sst/etc/ for
	linux and qnx) and restart the syslogd.  It should be changed 
	also the script makecd.sh in /usr/local/sst/scripts/ and
	reinstall it with make makecd.

	There is an "apparent" bug in QNX with syslogd.

	LOG_EMERGENCY seems do not work.  Then, I choose as the highest level ALERT
	LOG_LOCAL6 and LOG_LOCAL7 also seem do not work.  I choose then LOG_USER
	           and LOG_NEWS 

	Guigue - IAP, May 1998

*/

void log_msg( facility, msg, priority)
     unsigned short int priority, facility;
     char * msg;
{

  unsigned short int sysprio;

if ( ! (
       (facility & LOG_LOCAL0) | 
       (facility & LOG_LOCAL1) | 
       (facility & LOG_LOCAL2) |
       (facility & LOG_LOCAL3) | 
       (facility & LOG_LOCAL4) | 
       (facility & LOG_LOCAL5) |
       (facility & LOG_LOCAL6  )
       ) ){
  log_msg(LOG_LOCAL1,"log_msg: wrong facility definition",NOTICE);
  printf("\nlog_msg: wrong facility definition\n");
  return ;
}

if ( priority > LOG_DEBUG ) {
  log_msg(LOG_LOCAL1,"log_msg: wrong priority definition",NOTICE);
  return ;
}

sysprio  = facility | priority ; 
syslog( priority  ,"%s", msg )        ;

}


