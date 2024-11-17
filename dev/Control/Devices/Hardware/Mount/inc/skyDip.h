#ifndef SKYDIP
#define SKYDIP



int print_usage()
{
	printf("\n\n  skyDip - Sky Dip for evaluation of atmosferic opacity.\n\n\n");
	
	printf("SYNOPSIS\n\n");
	
	printf(" skyDip [OPTIONS]\n\n");

	printf("DESCRIPTION\n\n");

	printf(" This program aims to carry out the assessment of atmospheric opacity, performing a sky sweep. 	\n");
	printf(" Using horizontal coordinates, scanning is performed initially by collecting the azimuth where 	\n");
	printf(" the telescope is located and performing an SLEW starting with 10 degrees of elevation with the	\n");
	printf(" current azimuth fixed. After reaching this target, it starts a step-by-step sweep, keeping the \n");
	printf(" azimuth fixed and varying the elevation. The default values, ie without passing any options, 	\n");
	printf(" define two parameters: Step and Time, where step equals 10 degrees and 10 seconds time. While 	\n");
	printf(" the telescope is in position, it is given an op_mode = SLEW tag. Once in position, it changes 	\n");
	printf(" the mark to op_mode = SKY_DIP and will wait 10 seconds by default to perform the opacity 	\n");
	printf(" measurement and then continue to the next elevation considering a 10 degree step, again changing \n");
	printf(" the mark to op_mode = SLEW, and this process is repeated until Elevation is 90 degrees. After 	\n");
	printf(" finishing this scan, the telescope stops the scan and marks it with the tag op_mode = STALL. 	\n");
	printf(" It is also possible to modify the start and end elevation coordinates through the --el_init 	\n");
	printf(" and --el_end options, although if not entered, by default the respective values will be 10 	\n");
	printf(" degrees and 90 degrees respectively.								\n\n");

	printf("OPTIONS\n\n");

	printf(" -h, --help		Print this help and exit.\n");
	printf(" -v, --verbose		Verbose option. Basic output informations.\n");
	printf(" --debug		Full output information from skyDip and log.\n");
	printf(" --version		Show skyDip's version and exit.\n");
	printf(" -s, --step		Modify the step value in degrees. Default value is 10 degrees if not specified.\n");
	printf(" -t, --time		Modify the time value in seconds. Default value is 10 seconds if not specified.\n");
	printf(" --el_init		Modify the initial elevation coordinate to perfom the scan in the sky.\n");
	printf("			Default value is 10 degrees if not specified.\n");
	printf(" --el_end		Modify the final elevation coordinate to perfom the scan in the sky.\n");
	printf("			Default value is 90 degrees if not specified.\n\n");
	printf(" --az_offset		Modify de Azimute offset value adding to current azimute coordinate.\n\n");
 	printf("                        Default value is 5 degrees if not specified.\n\n");


	printf("EXAMPLES\n\n");

	printf(" Regular skyDip:   $ ./skyDip\n\n");

	printf(" Explanation - The command above perform the sky dip from actual azimute coordinate and starts making	\n");
	printf(" a slew to elevation 10 degrees, with time waiting of 10 seconds per step, increesing the elevation 	\n");
	printf(" in steps of 10 degrees, until the max elevation of 90 degrees. Leaving the telescope in STALL mode.	\n\n");

	printf(" Custom skyDip:	   $ ./skyDip -s 5 -t 7 --el_init 20 --el_end 70 --az_offset 10 -v\n\n");
	
	printf(" Explanation - The command above perform the sky dip from an offset azimute of 10 from the current      \n"); 
	printf(" azimute coordinate and starts making a slew to elevation 20 degrees, with time waiting of 7 seconds    \n");
	printf(" per step, increesing the elevation in steps of 5 degrees, until the max elevation of 70 degrees.       \n");
	printf(" Leaving the telescope in STALL mode.	\n\n");

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


