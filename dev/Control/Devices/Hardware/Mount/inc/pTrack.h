#ifndef PTRACK
#define PTRACK



int print_usage()
{
	printf("\n\n  pTrack - Point program to set observation target and Tracking.\n\n\n");
	
	printf("SYNOPSIS\n\n");
	
	printf(" pTrack [OPTIONS] [OBJECT]\n\n");

	printf("DESCRIPTION\n\n");

	printf(" The pTrack program sets the observation first by pointing the target, immediately after that,\n");
	printf(" the mount starts to do the slew by moving itself to the target. When arrived at position, the \n");
	printf(" TheSkyX system gets the tracking parameters based on selectec object and then starts to \n");
	printf(" tracking the object, keeping it centered in the field. The software ends its execution by\n");
        printf(" returning the information that the selected object is being tracking.\n\n");

	printf(" Information about objects:\n\n");
	
	printf(" This telescope (HATS) was built specificaly to solar research, which its Terahertz technology\n");
	printf(" has a 'field of view' or 'beam' (radio term) about of 11 arcsec. Thus the Sun (about 1900 arcsec)\n");
	printf(" but the detector have a 10mm one pixel aperture, for a solar image size of 9,5mm. Because of this\n");
	printf(" caracteristics for this application, is not useful at this moment, a pointing system to target\n");
	printf(" active regions on the Sun, even though	the mount having a periodic error of 7 arcsec peak-to-peak.\n\n");

	printf("OBJECTS\n\n");

	printf(" Common objects for use: sun moon mercury venus mars jupiter saturn uranus neptune pluto\n");
	printf(" Special objects: sky AR PARK HOME STALL\n\n");
	printf("        sky     move the telescope to a offset position to scape from object to a background sky temperature.\n");
	printf("        AR      move the telescope to a Active Region on the sun.\n");
	printf("        PARK    move the telescope to the park position - this stops tracking.\n");
	printf("        HOME    move the telescope to find the home position.\n");
	printf("        STALL  	stop telescope tracking - this stops tracking and don't move the mount.\n\n");

	printf("OPTIONS\n\n");

	printf(" -h, --help		Print this help and exit.\n");
	printf(" -p, --park		Park the telescope. The Object 'PARK' already park the telescope, so the option -p is optional.\n");
	printf(" -u, --unpark		UnPark the telescope. If combined with an object, you can use it to unpark and slew to an objetct.\n");
	printf(" -c, --connect		Connect to the telescope.\n");
	printf(" -d, --disconnect	Disconnect the telescope.\n");
	printf(" -f, --findhome		Move the telescope to Home position, necessary when mount turned the power on.\n");
	printf(" -pd			Park and Disconnect the telescope.\n");
	printf(" --joy			Enable joystick by arrows (^,<,>,v) or keys:\n");
	printf("					W = north\n");
	printf("					A = west\n");
	printf("					D = east\n");
	printf("					S = south\n\n");

	printf("The following manual coordinate options expect decimal (positive or negative) values:\n\n");
	
	printf("Equatorial System:\n");
	printf(" --ra <+00.000000>		Enable manual input RA (Right Ascention) coordinates.\n");
	printf(" --dec <+00.000000>		Enable manual input Declination coordinates.\n\n");

	printf("Horizontal System:\n");
	printf(" --az <+00.000000> 		Enable manual input Azimute coordinates.\n");
	printf(" --el <+00.000000> 		Enable manual input Elevation coordinates.\n\n");

	printf("	Example for manual equatorial coordinates input for slew to the Sun:\n");
	printf("	    	./pTrack --ra 12.7247 --dec -4.6740\n\n");

	printf(" --stall		Stops telescope tracking. The same of STALL object parameter.\n");
	printf(" --opmode		Show the Operation Mode status from Shared Memory.\n");
	printf(" -v, --verbose		Verbose option. Basic output informations.\n");
	printf(" --debug		Full output information from pTrack and log.\n");
	printf(" --version		Show pTrack's version and exit.\n\n");

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



//----------------------------------------------
#endif


