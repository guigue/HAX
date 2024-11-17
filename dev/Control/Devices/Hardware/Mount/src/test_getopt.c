//TESTE DE GETOPT
//
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

//Opt Flag Variables
static int version_flag		;
static int debug_flag		;
static int opmode_flag		;
static int joy_flag		;

int ra_flag 		= 0	;
int dec_flag 		= 0	;
int az_flag 		= 0	;
int el_flag 		= 0	;

int help_flag 		= 0	;
int park_flag 		= 0	;
int unpark_flag 	= 0	;
int connect_flag 	= 0	;
int disconnect_flag 	= 0	;
int verbose_flag	= 0	;

//Opt Arguments
char *ra_value 		= NULL	;
char *dec_value 	= NULL	;
char *az_value 		= NULL	;
char *el_value 		= NULL	;
char *object		= NULL	;

int optflag_ctr		= 0	;	// Options Flag counter



int main (int argc, char **argv)
{
	int c;

	while (1)
	{
		static struct option long_options[] =
		{
			/* These options set a flag. */
			{"version", no_argument,       	&version_flag, 	1},
			{"debug",   no_argument,       	&debug_flag,   	1},
			{"opmode",  no_argument,       	&opmode_flag,  	1},
			{"joy",     no_argument,       	&joy_flag,     	1},
			/* These options dont't set a flag. We distinguish them by their indices. */
			{"ra",	required_argument, 	0,	0},
			{"dec",	required_argument, 	0,   	0},
			{"az",	required_argument, 	0,   	0},
			{"el",	required_argument, 	0,   	0},
			{"help",    	no_argument,	0, 	'h'},
			{"park",   	no_argument, 	0, 	'p'},
			{"unpark", 	no_argument, 	0, 	'u'},
			{"connect", 	no_argument, 	0, 	'c'},
			{"disconnect",	no_argument, 	0, 	'd'},
			{"verbose", 	no_argument,   	0, 	'v'},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "hpucdv", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				if (memcmp( long_options[option_index].name,"ra",2)==0)
				{	
					ra_flag = 1;
					optflag_ctr++;
					printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					{ ra_value = optarg;
					       	printf("%s\n",ra_value); }
				}
				if (memcmp( long_options[option_index].name,"dec",3)==0)
				{	
					dec_flag = 1;
					optflag_ctr++;
					printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					{ dec_value = optarg; 
						printf("%s\n",dec_value); }
				}
				if (memcmp( long_options[option_index].name,"az",2)==0)
				{	
					az_flag = 1;
					optflag_ctr++;
					printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					{ az_value = optarg; 
						printf("%s\n",az_value); }
				}
				if (memcmp( long_options[option_index].name,"el",2)==0)
				{	
					el_flag = 1;
					optflag_ctr++;
					printf("%s_flag: ", long_options[option_index].name);
					if (optarg)
					{ el_value = optarg; 
						printf("%s\n",el_value); }
				}
				break;
			case 'h':
				help_flag = 1;
				optflag_ctr++;
				printf("help_flag: %d\n",help_flag);
				break;
			case 'p':
				park_flag = 1;
				optflag_ctr++;
				printf("park_flag: %d\n",park_flag);
				break;
			case 'u':
				unpark_flag = 1;
				optflag_ctr++;
				printf("unpark_flag: %d\n",unpark_flag);
				break;
			case 'c':
				connect_flag = 1;
				optflag_ctr++;
				printf("connect_flag: %d\n",connect_flag);
				break;
			case 'd':
				disconnect_flag = 1;
				optflag_ctr++;
				printf("disconnect_flag: %d\n",disconnect_flag);
				break;
			case 'v':
				verbose_flag = 1;
				optflag_ctr++;
				printf("verbose_flag: %d\n",verbose_flag);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				exit(1);
				break;
			default:
				abort ();
		}
	}
	/* Instead of reporting ‘--verbose’
	 * and ‘--brief’ as they are encountered,
	 * we report the final status resulting from them.
	 */
	if (version_flag)
		printf("version_flag: %d\n", version_flag);
	if (debug_flag)
		printf("debug_flag:   %d\n", debug_flag);
	if (opmode_flag)
		printf("opmode_flag:  %d\n", opmode_flag);
	if (joy_flag)
		printf("joy_flag:     %d\n", joy_flag);
	
	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		int diff = 0;
		object = argv[optind];
		diff = argc - optind;
		printf ("non-option ARGV-elements: ");
		while (optind < argc)
			printf ("%s ", argv[optind++]);
		if (diff > 1)
		{
			printf("\n");		
			puts("pTrack: Too many arguments for object. Try -h or --help.");
			exit(1);
		}
		printf("\rObject = %s                 \n",object);
	}
	if (argc < 2)
	{	
		puts("pTrack: Too few arguments or options. Try -h or --help.");
		exit(1);
	}
	
	if (optind > 1 && optflag_ctr == 0 && !object)
	{	
		puts("pTrack: Too few or incomplete arguments. Try -h or --help.");
		exit(1);
	}
	
	if (object && memcmp(object,"-",1) == 0)
	{
		puts("pTrack: Object Sintax Error! Try -h or --help.");
		exit(1);
	}	

	if (help_flag == 1 && (argc > 2 || optflag_ctr > 1))
	{	
		puts("pTrack: If you need help, don't input more options or parameters! Try -h or --help.");
		exit(1);
	}

	if (joy_flag == 1 && argc > 2)
	{	
		puts("pTrack: The Joystick fuction must be used without another option! Try -h or --help.");
		exit(1);
	}
	
	if (opmode_flag == 1 && argc > 2)
	{
		puts("pTrack: For opmode information must be used without another option! Try -h or --help.");
		exit(1);
	}

	if (version_flag == 1 && argc > 2)
	{
		puts("pTrack: For version information must be used without another option! Try -h or --help.");
		exit(1);
	}

	if ((connect_flag == 1 && argc > 2) && (connect_flag == 1 && verbose_flag != 1 && debug_flag != 1))
	{
		puts("pTrack: Options denied! Connect option only can be combined with -v or --debug options! Try -h or --help.");
		exit(1);
	}

	if (park_flag == 1 && unpark_flag == 1)
	{
		puts("pTrack: Options denied! You must decide before if you want park or unpark the telescope. Try -h or --help.");
		exit(1);
	}

	if (park_flag == 1 && object && memcmp(object,"PARK",4) != 0)
	{
		puts("pTrack: Options denied! You need to Park or observe something? Try -h or --help.");
		exit(1);
	}

	if ((disconnect_flag == 1 && object && memcmp(object,"PARK",4) != 0) )
	{ 
		puts("pTrack: Options denied! Disconnect option only can be combined with --debug, -v or -p options or PARK object. Try -h or --help.");
		exit(1);
	}



	printf("Run the pTrack instructions!\n");
	//printf("optind= %d\n",optind);
	//printf("argc= %d\n",argc);
	//printf("optflag_ctr= %d\n",optflag_ctr);
	exit (0);
}
