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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "hiddev.h"
#include "utils.h"
#include "options.h"
#include "contour-protocol.h"

int main(int argc, char *argv[])
{
	FILE *outf;
	struct user_options opts;
	struct msg msg;
	int fd, usage_code, ret, error;
	int entries = 0;

	read_args(argc, argv, &opts);

	trace_level = opts.trace_level;

	if (opts.output_path) {
		outf = fopen(opts.output_path, "w");
		if (outf == NULL) {
			error = errno;
			trace(0, "Failed to open output file %s: %s\n",
				opts.output_path, strerror(error));
			return 1;
		}
	} else {
		outf = stdout;
	}

	if (opts.usbdev == NULL)
		fd = wait_for_device(CONTOUR_USB_VENDOR_ID,
				CONTOUR_USB_PRODUCT_ID, &usage_code);
	else
		fd = hiddev_open(opts.usbdev, &usage_code);
	if (fd < 0)
		return 1;

	trace(0, "Initializing\n");
	contour_initialize(fd, usage_code);

	trace(0, "Done! Reading data\n");
	while (1) {
		ret = contour_read_entry(fd, usage_code, &msg);
		sanitize_ascii(msg.data, ret);

		if (ret < 45)
			break;

		fprintf(outf, "%s\n", msg.data);

		entries++;
		if (outf != stdout) {
			trace(0, "\r%d", entries);
			fflush(stdout);
		}
	}
	trace(0, "\n");

	return 0;
}
