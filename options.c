#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "options.h"

int read_args(int argc, char *argv[], struct user_options *opts)
{
        int option_index = 0, c;
        static struct option long_options[] = {
                { .val = 'd', .name = "device", .has_arg = 1, },
                { .val = 'v', .name = "verbose", .has_arg = 2},
        };
        char short_options[] = "d:v";

	memset(opts, 0, sizeof(*opts));

        while (1) {
                c = getopt_long(argc, argv, short_options, long_options,
                                &option_index);

                if (c == -1)
                        break;

                switch (c) {
		case 'd':
			opts->usbdev = optarg;
			break;
		case 'v':
			if (optarg)
				opts->trace_level = atoi(optarg);
			else
				opts->trace_level++;
                case '?':
			return -1;
                }
        }

        while (optind < argc) {
		/*
		 * Some day we do something useful here with the rest
		 * of the options.. Maybe
		 */
		optind++;
        }

	return 0;
}
