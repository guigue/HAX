#ifndef SCANTAR
#define SCANTAR



int print_usage()
{
	printf("\n\n  scanTar - Scan Target for getting the energy curve of an object.\n\n\n");
	
	printf("SYNOPSIS\n\n");
	
	printf(" scanTar [OPTIONS]\n\n");

	printf("DESCRIPTION\n\n");

	printf(" This program performs a straight scan over a target being observed and tracked. As a premise,	\n"); 
	printf(" this command expects that some object is already being observed, and the telescope must be in	\n");
	printf(" tracking mode. When this command is executed, it is initially offset from the current position \n");
	printf(" and then starts scanning from the sky through the object and then ending at the sky again. 	\n");
	printf(" After performing N passes through the object, a value that can be parameterized, the telescope \n");
	printf(" points to the object again and follows it.							\n\n");

	printf("OPTIONS\n\n");

	printf(" -h, --help		Print this help and exit.\n");
	printf(" -v, --verbose		Verbose option. Basic output informations.\n");
	printf(" --debug		Full output information from scanTar and log.\n");
	printf(" --version		Show scanTar's version and exit.\n");
	printf(" -o, --offset		Modify the inicial distance from the object to perform the scan.\n");
	printf("			Default value is 0.0 arcmin, if not specified. This means that the FOV\n");
	printf("			circle of telescope tangent to the object limb.\n");
	printf(" -n, --nscan		Specifies the quantity of scan to be performed.\n");
	printf("			Default value is 1 (one) if not specified.\n");
	printf(" -s, --step		Specifies the sub-slews between scan distance limits. This is a way to\n");
	printf("			control the scan velocity, when the default value is defined as a 1/20\n");
	printf("			of scan travel, if not specified. The input must be in arc-minutes.\n");
	printf(" -t, --type		Defines the type of scan, when RA = 1, DEC = 2, AZ = 3 and EL = 4.\n");
	printf("			If not specified, the default value is 1.\n\n");

	printf("EXAMPLES\n\n");

	printf(" Regular scanTar:   $ ./scanTar      or      $ ./scanTar -v						\n\n");

	printf(" Explanation - The command above perform the scan on the actual object that is observed. With the	\n");
	printf(" option -v (--verbose) this shows some information about what is happen in the moment. For default 	\n");
	printf(" the parameters will perform only one pass in front of object and then back to observing the object.	\n\n");

	printf(" Custom scanTar:    $ ./scanTar -v -n 50 -o 58 -s 2.5 -t 1						\n\n");
	
	printf(" Explanation - The command above is a example of a solar scan. Considering the object angular size  	\n");
	printf(" of the sun with 30 arcmin and the FOV size of 34 arcmin, by ((fov + ((fov/2)-(objsize/2))) + offset) 	\n"); 
	printf(" was considerated an inicial total offset from RA (-t 1) coordinate of 90 arcmin or 1.5 degrees.	\n");
	printf(" The step option of 2.5 arcmin means that from total scan travel of 2*90=180 arcmin divided by   	\n");
	printf(" 2.5 arcmin, the scan will perform -n 50 times, 72 steps each, in front of the Sun.			\n\n");

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


