/*
 * Copyright (C) 2012 Timo Kokkonen <timo.t.kokkonen@iki.fi>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#include <getopt.h>
#include <string.h>
#include <stdlib.h>

#include "options.h"

int read_args(int argc, char *argv[], struct user_options *opts)
{
        int option_index = 0, c;
        static struct option long_options[] = {
                { .val = 'd', .name = "device", .has_arg = 1, },
                { .val = 'v', .name = "verbose", .has_arg = 2 },
		{ .val = 'o', .name = "output", .has_arg = 1 },
        };
        char short_options[] = "d:v:o:";

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
		case 'o':
			opts->output_path = optarg;
			break;
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
