/******************************************************************************

                                  modes.h

*******************************************************************************/


/********************** MODES OF ANTENNA MOVEMENT *****************************/


/* MODES FOR ANTENNA MOVEMENT:

         0 - tracking a target
	 1 - Mapping a squared region centered on Target: in 
	     Righ x Decl with the size and step in Declination 
	     defined in struct antenna (f.ex.: for solar maps
	     40x40 arcmin or Active Regions 4x4 acmin)
        21 - as 1 but fast axis in Decl.
	 2 - Mapping a squared region centered on Target: in 
	     Azim. x Elev. with the size and step in Elevation
	     defined in struct antenna (f.ex: for solar maps)
        22 - as 2 but fast axis in Elev.
	 3 - Mapping a Target radially (odd  numbered maps)
        23 - Mapping a Target radially (even numbered maps)
         4 - intermediate mapping mode (acceleration/deceleration times)
	 5 - Scanning the Target in Azimuth (size,time interval->antenna)
	 6 - Scanning the Target in Elevation (size,time interval->antenna)
	 7 - Scanning the Target in Right Ascention (    "		)
	 8 - Scanning the Target in Declination (      "		)
	 9 - intermediate scanning mode (acceleration/deceleration times)
	10 - Scanning the sky in elevation (0-90) +10 deg off in az for Tau.
             SCAN_TAU will use target SKY
	11 - Scanning the Target in RA/DEC under an angle
	12 - Scanning the Target in AZ/EL  under an angle
	14 - Fast movement during the bursts 
	15 - On-Off tracking of the target (time interval, off)
	16 - On-On tracking of the target switching beams (time interval, off)
	17 - Off tracking the target for the onoff mode 
    18 - On-Off/On intermediate mode
	50 - Stall mode. Mode where the antenna stops where it is. (stop)
        99 - undefined mode of operation between two modes                  */

#define TRACK		 0   

#define MAP_RADEC	 1
#define MAP_DECRA	21
#define MAP_AZEL	 2
#define MAP_ELAZ	22
#define MAP_RADIAL1	 3
#define MAP_RADIAL2	23
#define MAP_INTERM       4

#define SCAN_AZ		 5
#define SCAN_EL		 6
#define SCAN_RA		 7
#define SCAN_DEC	 8
#define SCAN_INTERM      9

#define SCAN_TAU	10
#define SCAN__RADEC	11
#define SCAN__AZEL      12
#define FAST_8SCAN	14

#define ONOFF		15
#define ONON		16
#define OFFPOINT	17		// off position in azimuth for onoff mode
#define ON_INTERM	18      // intermediate mode for on-off or on-on modes

#define MAX_ATT	        40   // maximum attenuation 31dB 

#define STALL		50
#define ANT_LOCKED	55   // problem with antenna
#define UNKNOWN_MODE	99  



