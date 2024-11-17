//TESTE DE GETOPT
//
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

/* Flag set by ‘--verbose’. */
static int verbose_flag;

int main (int argc, char **argv)
{
	int c;

	while (1)
	{
		static struct option long_options[] =
		{
			/* These options set a flag. */
			{"verbose", no_argument,       &verbose_flag, 1},
			{"brief",   required_argument, 0, 0},
			{"brasil", required_argument, 0, 0},
			/* These options don’t set a flag. We distinguish them by their indices. */
		/*	{"add",     no_argument,       0, 'a'},  */
			{"append",  no_argument,       0, 'a'},
			{"delete",  required_argument, 0, 'd'},
			{"create",  required_argument, 0, 'c'},
			{"file",    required_argument, 0, 'f'},
			{0, 0, 0, 0}
		};
		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "abc:d:f:", long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				if (memcmp( long_options[option_index].name,"brief",6)==0)
				{	
					printf ("ooption %s", long_options[option_index].name);
					if (optarg)
					printf (" with arg %s", optarg);
				}
				printf ("\n");
				if (memcmp( long_options[option_index].name,"brasil",7)==0)
				{	
					printf ("ooption %s", long_options[option_index].name);
					if (optarg)
					printf (" with arg %s", optarg);
				}
				printf ("\n");
				break;
			case 'a':
				puts ("option -a\n");
				break;
			case 'b':
				puts ("option -b\n");
				break;
			case 'c':
				printf ("option -c with value `%s'\n", optarg);
				break;
			case 'd':
				printf ("option -d with value `%s'\n", optarg);
				break;
			case 'f':
				printf ("option -f with value `%s'\n", optarg);
				break;
			case '?':
				/* getopt_long already printed an error message. */
				break;
			default:
				abort ();
		}
	}
	/* Instead of reporting ‘--verbose’
	 * and ‘--brief’ as they are encountered,
	 * we report the final status resulting from them.
	 */
	if (verbose_flag)
	{
		printf("verbose flag is set: %d\n", verbose_flag);
		printf("...with arg value: %s\n",  optarg);
	}
	/* Print any remaining command line arguments (not options). */
	if (optind < argc)
	{
		printf ("non-option ARGV-elements: ");
		while (optind < argc)
			printf ("%s ", argv[optind++]);
		putchar ('\n');
	}
	exit (0);
}

