/************************************************************************

 			      TARGET.C				       
 								       
  Sets the observation target to a chosen object, sets mode to track   
  and the calibration mirror to ANTENNA.			       
 								       
  After checking all parameters target sets the mode to UNKNOWN_MODE,  
  and then initiates the antenna to move to the new object: the programs 
  coord and point will then move the antenna to point to the new target. 
  As soon as it is reached the mode is set to the mode TRACK.			                     

  To interrupt target CONTROL C is used and the target will stop with
  the following steps:
  
    - Set the mode to UNKNOWN_MODE
    - Save the target temporarily and set the target to MANUAL
    - Read the actual encoder position and command the antenna to that 
      position, ie, stop
    - Wait until the antenna stops (error in position < TRACK_ERR)
    - Change target back to targeted object and mode to STALL
    
    
    Note: coord sets target to sun and mode to STALL as default
 								       
 	       J.E.R.Costa and E.Rolli  			       
 
 	       precession added by Guigue			       
 
    CHANGES by A. Magun: send_object sends only target and does not include
                         calibration mirror position anymore ->
			 send_target has become obsolete.
             J.E.R.Costa: keyword -l added to input offsets in 
                          AZIMUTH and ELEVATION instead of RA, DEC
			 
************************************************************************/
#ifdef __USAGE
         target - SST User-Interface program to set observation target

SYNOPSIS
   target object [object_dependent_parameters]

   object : sun  AR  moon  star  sky    (AR = Active Region on the sun)
            beacon  service  manual
            mercury venus mars jupiter saturn uranus neptune pluto

DESCRIPTION
   The target program sets the observation target to the chosen object, 
   the observation mode to TRACK and the calibration mirror to ANTENNA.

PARAMETERS
   - target sun  [offset_RA offset_DEC]        solar rotation not corrected
   - target -l sun  [offset_AZ offset_EL]      
   - target AR   [offset_RA offset_DEC [UT]]   solar rotation corrected
   - target -l AR   [offset_AZ offset_EL [UT]]   solar rotation corrected
   - target moon [offset_RA offset_DEC]
   - target -l moon [offset_RA offset_DEC]
   - target star RA DEC [year [catalog]]
   - target sky                                azimuth is offset from
                                               current by 10 degres                                               
                                               object + 10 deg

   - target beacon                             point to beacon position
   - target service                            point to service position
   - target manual [azimuth elevation]         point to azimuth and elevation

   - target "planet" [offset_RA offset_DEC]
   - target -l "planet" [offset_AZ offset_EL]

   offset_RA and offset_DEC   are in arcseconds           (defaults are 0)
   or
   offset_AZ and offset_EL    are in arcseconds           (defaults are 0)
   UT                         UT-time in the format hh:mm (defaultis 0 UT)
   RA and DEC                 are in hours and degrees
   year                       year of the catalog e.g. 1950
                              (default is today with no conversion of RA/DEC)
   catalog                    FK5 (default) or FK4
   Azimuth and Elevation      are in degrees

#endif

#include <math.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <conio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <i86.h>
#include <termios.h>
#include <mqueue.h>
#include <syslog.h>
#include <process.h>
#include <unistd.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/proxy.h>
#include <sys/kernel.h>
#include <sys/sched.h>
#include <sys/dev.h>
#include <sys/psinfo.h>
#include <sys/name.h>

#include "atmio16x.h"
#include "files.h"
#include "buffers.h"
#include "modes.h"
#include "targets.h"
#include "ephemer.h"
#include "ephem_lib.h"
#include "antenna.h"
#include "cal_mirror.h"

char     * prname = "target: " ;

extern void  log_msg( unsigned short int, char *, unsigned short int);
extern void  send_opmode(mqd_t, char) ; 
extern void  send_object(mqd_t, char);
extern mqd_t open_control_mqueue1() ;
extern void  open_shmem1() ;
extern void  send_cal_mirr_pos(mqd_t, short) ; 
extern void  log_mode( char *) ;

volatile struct_ephemer   *ephemer_ptr     ;
volatile struct_node_data *node1_data_ptr  ; 


mqd_t mqdes ;


/* Signal Handler: stop positioner at current position with target MANUAL */
void Signal_Handler(int sig_number){

        float dist = 100*TRACK_ERR ;
        int mem_targ ;          

	sigset_t set=0x4127;
	sigprocmask( SIG_BLOCK, &set, NULL );  // mask other signals
        mem_targ = (*ephemer_ptr).coord.target ;
	(*ephemer_ptr).coord.target = MANUAL ;
	send_object(mqdes, (*ephemer_ptr).coord.target) ;

	(*ephemer_ptr).coord.azim = (*ephemer_ptr).coord.enc_az ;
	(*ephemer_ptr).coord.elev = (*ephemer_ptr).coord.enc_el ;


	(*ephemer_ptr).coord_comp.mode = UNKNOWN_MODE ;
	send_opmode(mqdes,(*ephemer_ptr).coord_comp.mode); 



	log_mode("");				// write to mode/target-log

	/* wait until tracking point is reached */
	delay(100);
	while(dist > TRACK_ERR ) {
	   dist = sqrt(pow((*ephemer_ptr).coord.az_err,2) +
			pow((*ephemer_ptr).coord.el_err,2) ) ;
	   delay(10);
	}

	/* set mode to track */
	(*ephemer_ptr).coord_comp.mode = STALL ;
	send_opmode(mqdes,(*ephemer_ptr).coord_comp.mode); 
	(*ephemer_ptr).coord.target = mem_targ ;
	send_object(mqdes, (*ephemer_ptr).coord.target) ;
	log_mode("");				// write to mode/target-log


	log_mode(" target has been stopped ");
	log_msg(LOG_CTRL,"target has been stopped", ALERT);
	closelog();
	exit(0);
}




void main(int argc, char * argv[]) {

	int i, pid, err, pty ;
	float dist=360, days=0.0 ;
	double ra, dec, equinox1, equinox2 ;
	double w, P, B, R, s_lat, s_long ;
	double ut_ref, ra0, dec0;
	int    opt, nargc=1 ;
	char   *nargv[20];
    int local=0 ;

	char catalogo[3] ="FK5" ;
	char arg[80], msg[255] ;

	struct tm     gmt ;	 /* ut in year, day, .... seconds	*/
	struct timeb  timebuf ;  /* ut in s and ms from jan 1, 1970     */
	                         /* in timebuf.time and timebuf.millitm */ 
	int time_ms ;
	double jd0 ;

	typedef struct {
		int targets;
		char * keyword;
	} TARGETS_MAP;



	TARGETS_MAP keywordmap[]={
		SKY,		"sky",
		MERCURY,	"mercury",
		VENUS,		"venus",
		MARS,		"mars",
		JUPITER,	"jupiter",
		SATURN,		"saturn",
		URANUS,		"uranus",
		NEPTUNE,	"neptune",
		PLUTO,		"pluto",
		MOON,		"moon",
		SUN,		"sun",
		AR,			"AR",
		STAR,		"star",
		BEACON,		"beacon",
		SERVICE,	"service",
		MANUAL,		"manual",
		UNKNOWN_OBJECT, "unknown object"	};

	arg[0]='\0' ;


        /* First register the program to syslogd() */

        putenv("SYSLOG=1")                       ;
        openlog(prname, LOG_NDELAY, LOG_OTHER)    ;
        log_msg(LOG_CTRL,">>> executing",NOTICE);
	
	/* print usage */
        if (argc < 2) { 
            log_msg(LOG_EPHEM,"wrong number of arguments",ALERT);
            log_msg(LOG_CTRL,"<<< killed", EMERGENCY);
	    closelog() ;
            print_usage( argv ); 
	    exit(1) ;
	}

	/* separation of options from arguments: nargc and nargv correspond */
	/* to argc and argv without options and option-arguments            */
	while ( optind < argc ) {
	    opt=getopt(argc,argv,"l0123456789.");
	    switch(opt) {
		case 'l':
		    local=1;        // the offsets given are in azimuth and elevat.
		    break;
		default:
		    nargv[nargc++]=argv[optind++];
		    break;
	    }
	}
//    printf("nargc: %d  nargv[1]: %s \n",nargc,nargv[1]) ;   

	/* find target */
	for (i=0; keywordmap[i].targets !=UNKNOWN_OBJECT ; i++){
	   if(strnicmp(nargv[1],keywordmap[i].keyword,3) == 0) break; }

	/* catch wrong target */
	if (keywordmap[i].targets == UNKNOWN_OBJECT) {
            log_msg(LOG_EPHEM,"unknown object",ALERT);
            log_msg(LOG_CTRL,"<<< killed", ALERT);
	    closelog() ;
            print_usage( argv ); 
	    exit(1) ;
	}


	/* check for errors in input arguments */
	switch(keywordmap[i].targets){
	  case STAR:
	    if ((nargc < 4) || (nargc > 6) || (local == 1)) {
               log_msg(LOG_EPHEM,"wrong number of arguments",ALERT);
               log_msg(LOG_CTRL,"<<< killed", EMERGENCY);
	       closelog() ;
               print_usage( argv ); 
	       exit(1) ;
            }
	    break ;
	  case AR:
// printf("aqui  %d %s %s %s \n",nargc,nargv[0],nargv[1],nargv[2]) ;
	    if ((nargc < 4) && (nargc != 2)) {
               log_msg(LOG_EPHEM, "wrong number of arguments",ALERT);
               log_msg(LOG_CTRL,"<<< killed", EMERGENCY);
               print_usage( argv ); 
	       closelog() ;
	       exit(1) ;
            }
	    break ;
	  case MANUAL:
	    if ((nargc != 4) && (nargc != 2) || (local == 1)) {
               log_msg(LOG_EPHEM, "wrong number of arguments",ALERT);
               log_msg(LOG_CTRL,"<<< killed", EMERGENCY);
	       closelog() ;
               print_usage( argv ); 
	       exit(1); 
	    }
	    break ;
	  case SERVICE:
	  case BEACON:
      case SKY:
	    if ((nargc != 2) || (local ==1)) {
               log_msg(LOG_EPHEM, "wrong number of arguments",ALERT);
               log_msg(LOG_CTRL,"<<< killed", EMERGENCY);
	       closelog() ;
               print_usage( argv );          
	       exit(1) ;
            }
	    break ;
	  default:
	    if ((nargc != 4) && (nargc != 2)){
               log_msg(LOG_EPHEM, "wrong number of arguments",ALERT);
               log_msg(LOG_CTRL,"<<< killed", EMERGENCY);
	       closelog() ;
               print_usage( argv );
	       exit(1);
	    }
	}
        
        /* open log window */
        pty = open("//1/dev/ttyp0",O_WRONLY) ;


	/* open shared memory & mqueue */
        open_shmem1();
	mqdes = open_control_mqueue1() ;


	/* - Kill other UI-process
	 * - setup the signal handler: When ever the process is kill 
	 *   (kill, slay or Ctrl-C) the target is set to manual with the
	 *   actual azimuth and elevation.
	 * - register as the current UI-process
	 */
	pid=qnx_name_locate( 0, "sst/user-interface", 0, NULL );
	if (pid != -1) { kill(pid, SIGTERM); }
	/* wait until process finished */
	while (qnx_name_locate( 0, "sst/user-interface", 0, NULL ) != -1){
	  delay(1);
	}

	signal( SIGTERM, Signal_Handler) ;	// catch slay, kill
	signal( SIGINT,  Signal_Handler) ;	// catch Ctrl-C
	signal( SIGQUIT, Signal_Handler) ;	// catch kill -QUIT
	signal( SIGABRT, Signal_Handler) ;	// catch abort
	signal( SIGHUP,  Signal_Handler) ;	// catch hangup
	signal( SIGUSR1, Signal_Handler) ;	// catch user signal1: reset_ui
	qnx_pflags( ~0,_PPF_IMMORTAL ,0,0) ;	// allows KILL to be caught
	signal( SIGKILL, Signal_Handler) ;	// catch kill -KILL


	/* attach the name "sst/user-interface" to the current process */
	qnx_name_attach(0 , "sst/user-interface" );



	/******* now start  *******/

	/* Reset everything to start tracking the center of the target */
	(*ephemer_ptr).coord_comp.act_off_ra  = 0.0 ;
	(*ephemer_ptr).coord_comp.act_off_dec = 0.0 ;
	(*ephemer_ptr).coord_comp.act_off_az  = 0.0 ;
	(*ephemer_ptr).coord_comp.act_off_el  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_OFF_az  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_sky_az  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_AUX_az  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_AUX_el  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_AUX_ra  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_AUX_dec = 0.0 ;
	(*ephemer_ptr).coord_comp.off_RA_act  = 0.0 ;
	(*ephemer_ptr).coord_comp.off_DEC_act = 0.0 ;


	/* set mode to stall to avoid that the antenna is moving before */
	/*  every thing is set correctly 				*/
	(*ephemer_ptr).coord_comp.mode = STALL ;
	send_opmode(mqdes, STALL); 

	/* set new target and calibration mirror to antenna */
	(*ephemer_ptr).coord.target = keywordmap[i].targets ;
	send_object(mqdes, (*ephemer_ptr).coord.target);
	(*ephemer_ptr).calmirror.cmd_pos = ANT_POS ;
	send_cal_mirr_pos(mqdes, ANT_POS);

    delay(500) ;   // give some time to coord accept the target change
                   // otherwise coordinates can be change in the mean time
                   // for example for MANUAL
 

	/* special setting for the targets STAR and AR that need precession,
           offsets, differential rotation and so on */
	switch(keywordmap[i].targets){

	  case STAR:
	    /* --------- STAR --------- */
	    /* set RA and DEC if target is STAR   >>> RA IS IN HOURS <<<<   */

	    ftime( &timebuf ) ;			/* reads system real time */
	    _gmtime( &timebuf.time, &gmt ) ;

            equinox2 = 1900.0 + gmt.tm_year + gmt.tm_yday/ 365.25 ;

	    ra  = atodeg(nargv[2]) ;                /* Read RA */
	    dec = atodeg(nargv[3]) ;                /* Read Dec */
	    if (nargc >  4) equinox1 = atof(nargv[4]); else equinox1 = equinox2;
	    if (nargc == 6) strncpy(catalogo,strupr(nargv[5]),3);
	    print_equinox(ra, dec, equinox1, 0x0);
	    if (nargc > 4)
	      do_precess(&ra, &dec, equinox1, equinox2, catalogo);
	    print_equinox(ra , dec, equinox2, catalogo);

	    (*ephemer_ptr).coord.rect = ra  ;
	    (*ephemer_ptr).coord.decl = dec ;
	    sprintf( arg," %11.6f  %11.6f", 
		(*ephemer_ptr).coord.rect, (*ephemer_ptr).coord.decl);

	    break;


	  case AR:
	    /* --------- Active Region --------- */
	    if (nargc == 2) break;

	    ra  = atof(nargv[2]) ;
	    dec = atof(nargv[3]) ;
	    ut_ref = 0.0 ;
	    if ( nargc == 5 ) ut_ref = atodeg(nargv[4]) ;
        if ( nargc == 6 )  days  = atof(nargv[5])  ;
            log_msg(LOG_EPHEM, "",ALERT);
	    sprintf(msg, "\n\n Offsets:");
            write(pty, msg, strlen(msg)) ;  
            sprintf(msg,"\n         %8.2f\" (RA)", ra) ;
            write(pty, msg, strlen(msg)) ;  
            sprintf(msg,"\n         %8.2f\" (Dec)", dec) ;
            write(pty, msg, strlen(msg)) ;  	    
            sprintf(msg,"\n         %6.2f Hs (Reference Time)  \n\n", ut_ref) ;
            write(pty, msg, strlen(msg)) ;  	


	    /* Read time to calculate the sun rotation */
	    ftime( &timebuf ) ;			// reads system real time
	    time_ms  = (timebuf.time % 86400) * 1000 + timebuf.millitm ; 

//	    jd0=(double)((int)((*ephemer_ptr).ut_time.jd + 0.5) - 0.5) ;
	    jd0=(double)((int)((*ephemer_ptr).ut_time.jd + 0.5) - 0.5) - (double)days ;
        /* calculate the Sun inclination P and latitude of equator B and 
		radius R (arcsec) at 0 UT      */
	    solar_PB( jd0 , &P, &B, &R );

	    /* convert ra, dec (arcsec) to heliographic coord at 0 UT. */
        if(local != 1) { 
	    err = radec_hel(ra/60.0, dec/60.0, P, B, R/(double) 60, &s_lat, &s_long);
	    if (err != 0) {

              log_msg(LOG_EPHEM, "",ALERT);
              sprintf(msg,"\n         %6.2f Hs (Reference Time)  \n\n", ut_ref) ;	    
              write(pty, msg, strlen(msg)) ;  
              sprintf(msg," \33[1mCoordinates outside of solar disc\n") ;	    
              write(pty, msg, strlen(msg)) ;  
              sprintf(msg," Setting target to SUN \33[0m\n") ;	    
              write(pty, msg, strlen(msg)) ;  
 
	      (*ephemer_ptr).coord.target = SUN ;
	      send_object(mqdes, (*ephemer_ptr).coord.target);
	      (*ephemer_ptr).coord_comp.off_RA_act = 0.0 ; // offset in arcmin
	      (*ephemer_ptr).coord_comp.off_DEC_act= 0.0 ; // offset in arcmin
	      sprintf( arg," %11.6f  %11.6f",
		       (*ephemer_ptr).coord_comp.off_RA_act,
		       (*ephemer_ptr).coord_comp.off_DEC_act);
	      i--;
	      break;
	    }
        } else { 
        s_lat = ra ;
        s_long = dec ;
        }
        
	    /* solar rotation for latitude s_lat (degday) */
	    w = solar_rot(s_lat) ;  

	    /* convert to d_ra, d_dec(arcmin) at 0 UT. */
        if(days != 0.0) { 
           s_long = s_long + w*(days * 24.0 - ut_ref)/24.0 ;
           if(s_long > 90.0) {
              log_msg(LOG_EPHEM, "",ALERT);
              sprintf(msg,"\n         %6.2f Hs (Reference Time)  \n\n", ut_ref) ;	    
              write(pty, msg, strlen(msg)) ;  
              sprintf(msg," \33[1mCoordinates behind the visual solar disc\n") ;	    
              write(pty, msg, strlen(msg)) ;  
              sprintf(msg," Setting target to SUN\33[0m\n") ;	    
              write(pty, msg, strlen(msg)) ;  
 	          (*ephemer_ptr).coord.target = SUN ;
	          send_object(mqdes, (*ephemer_ptr).coord.target);
	          (*ephemer_ptr).coord_comp.off_RA_act = 0.0 ; // offset in arcmin
	          (*ephemer_ptr).coord_comp.off_DEC_act= 0.0 ; // offset in arcmin
	          sprintf( arg," %11.6f  %11.6f",
		         (*ephemer_ptr).coord_comp.off_RA_act,
		         (*ephemer_ptr).coord_comp.off_DEC_act);
	       i--;
	       break;
	       }
        } else  {
           s_long = s_long - w*(ut_ref/24.0) ;
        }

        /*         getting new P, B0 and R for today                       */
	    jd0=(double)((int)((*ephemer_ptr).ut_time.jd + 0.5) - 0.5) ;
	    solar_PB( jd0 , &P, &B, &R );

 	    hel_radec(s_lat , s_long,
			P, B, R / (double) 60, &ra0, &dec0) ;
 
            /* fill shmem with the d_ra, d_dec now and heliographics for 0 UT */
	    (*ephemer_ptr).coord_comp.off_RA_AR = ra0  ;  // arcmin
	    (*ephemer_ptr).coord_comp.off_DEC_AR= dec0  ;
	    sprintf( arg," %11.6f  %11.6f",
		(*ephemer_ptr).coord_comp.off_RA_AR,
		(*ephemer_ptr).coord_comp.off_DEC_AR);

	    /* save active region latitude and longitude*/
	    (*ephemer_ptr).coord_comp.AR_lat  = s_lat ;	
	    (*ephemer_ptr).coord_comp.AR_long = s_long ;	
	    break;


	  case MANUAL:
	    /* --------- MANUAL --------- */
            if (nargc == 2) break;         

	    /* fill shmem with the d_ra, d_dec  */
	    (*ephemer_ptr).coord.azim = atof(nargv[2]) ;
	    (*ephemer_ptr).coord.elev = atof(nargv[3]) ;
	    sprintf( arg," %11.6f  %11.6f",
		(*ephemer_ptr).coord.azim,
		(*ephemer_ptr).coord.elev);
	    break;


	  default:
	    /* --------- SUN, MOON, ... --------- */
            if (nargc == 2) break;     

	    /* fill shmem with the d_ra, d_dec  */
        if(local == 0) {
    	   (*ephemer_ptr).coord_comp.off_AUX_ra = atof(nargv[2]) / 60.0 ;
	       (*ephemer_ptr).coord_comp.off_AUX_dec= atof(nargv[3]) / 60.0 ;
	       (*ephemer_ptr).coord_comp.off_AUX_az = 0.0 ;
	       (*ephemer_ptr).coord_comp.off_AUX_el = 0.0 ;
           sprintf( arg," %11.6f  %11.6f",
		      (*ephemer_ptr).coord_comp.off_AUX_ra,
		      (*ephemer_ptr).coord_comp.off_AUX_dec);
        }
        if(local == 1) {
	       (*ephemer_ptr).coord_comp.off_AUX_az = atof(nargv[2]) / 60.0 ;
	       (*ephemer_ptr).coord_comp.off_AUX_el = atof(nargv[3]) / 60.0 ;
    	   (*ephemer_ptr).coord_comp.off_AUX_ra = 0.0 ;
	       (*ephemer_ptr).coord_comp.off_AUX_dec= 0.0 ;
	       sprintf( arg," %11.6f  %11.6f",
		      (*ephemer_ptr).coord_comp.off_AUX_az,
		      (*ephemer_ptr).coord_comp.off_AUX_el);
        }
	}



	/* --------- for ALL TARGETS ---------- */
	(*ephemer_ptr).coord_comp.mode = UNKNOWN_MODE ;
	send_opmode(mqdes,(*ephemer_ptr).coord_comp.mode); 
	log_mode("");


	/* -- now wait until antenna points to target and set then 
	      mode to TRACK -- */
	delay(300);
//	printf("\n\n");
	while(dist > TRACK_ERR ) {
	    dist = sqrt(pow((*ephemer_ptr).coord.az_err,2) +
			pow((*ephemer_ptr).coord.el_err,2) ) ;
//	    printf("\33M az_err: \33[1m %8.3f \33[0m",
//		   (*ephemer_ptr).coord.az_err);
//	    printf(" el_err: \33[1m %8.3f \33[0m dist: \33[1m%8.3f\33[0m\n",
//		   (*ephemer_ptr).coord.el_err, dist) ;
	    delay(100);
	}


        /* if target is not SERVICE set mode to TRACK (SERVICE: mode=STALL) */
	if (keywordmap[i].keyword != "service"){ 
	    (*ephemer_ptr).coord_comp.mode = TRACK ;
	    send_opmode(mqdes,(*ephemer_ptr).coord_comp.mode); 
	}

	/* write new target to observation log */
	log_mode(arg);

//	printf("\n Target is set to %s\n",keywordmap[i].keyword);
}			
		
	
