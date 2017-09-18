#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "options.h"

/* Flag set by --verbose */
static int verbose_flag;

static void print_example(int argc, char **argv){
	printf("\n");
	printf("EXAMPLE: \n");
	printf("\n");
	printf("    %s --turn 2.0 --accel 0.5 --decel 0.5 --speed 1.0\n", argv[0]);
	printf("\n");
}

static void print_usage(int argc, char **argv){
	print_example(argc, argv);
	printf("---------------------------------------------------------\n");
	printf("OPTION:\n");
	printf("     ?,                print usage and example message\n");
	printf("    -h, --help         print usage and example message\n");
	printf("    -x, --example      print details example\n");
	printf("    -t, --turn         total number of turn\n");
	printf("    -a, --accel        acceleration turn/sec*sec\n");
	printf("    -d, --decel        decceleration turn/sec*sec\n");
	printf("    -s, --speed        maximum speed turn/sec\n");
	printf("\n");
}

int get_motor_options(int argc, char **argv, struct motor_options *p)
{
	int c;
	char *endptr;

	if (argc == 1){
		/* just use default */
		print_usage(argc, argv);
		return 1;
	}

	while (1){
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"example", no_argument, 0, 'x'},
			{"turn", required_argument, 0, 't'},
			{"accel", required_argument, 0, 'a'},
			{"decel", required_argument, 0, 'd'},
			{"speed", required_argument, 0, 's'},
			{0, 0, 0, 0}
		};

		/* getopt_long stores the option index here. */
		int option_index = 0;

		c = getopt_long (argc, argv, "hx:t:a:d:s:", long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c){
			case 0:
				/* If this option set a flag, do nothing else now. */
				if (long_options[option_index].flag != 0)
					break;
				printf ("option %s", long_options[option_index].name);
				if (optarg)
					printf (" with arg %s", optarg);
				printf ("\n");
				return 0;
				break;

			case 'h':
				print_usage(argc, argv);
				return 0;
				break;

			case 'x':
				print_example(argc, argv);
				return 0;
				break;

			case 't':
				p->turn = atof(optarg);
				break;
			case 'a':
				p->accel = atof(optarg);
				break;

			case 'd':
				p->decel = atof(optarg);
				break;

			case 's':
				p->speed = atof(optarg);
				break;

			case '?':
				/* getopt_long already printed an error message. */
				break;

			default:
				print_usage(argc, argv);
				return 0;
				break;
		}
	}

	/* Instead of reporting ‘--verbose’
	   and ‘--brief’ as they are encountered,
	   we report the final status resulting from them. */

	if (verbose_flag)
		puts ("verbose flag is set");

	/* Print any remaining command line arguments (not options). */
	if (optind < argc){
		while (optind < argc){
			if (strcmp(argv[optind], "?")){
				printf ("\nUnknown option: %s\n", argv[optind]);
			}
			optind++;
		}
		putchar ('\n');

		/* print a usage message */
		print_usage(argc, argv);
		return 0;
	}

	return 1;
}
