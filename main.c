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

char *token(char **str, char sep);

int main(int argc, char *argv[])
{
	FILE *outf;
	struct user_options opts = { .usbdev = NULL, .output_path = NULL, .output_format = CLEAN, .trace_level = 0 };
	struct msg msg;
	int fd, usage_code, ret, error;
	int entries = 0;

	if ( read_args(argc, argv, &opts) )
		return -1;

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
	trace(0, "Initializing ...\n");
	contour_initialize(fd, usage_code);

	trace(0, "Reading data ...\n");
	if ( opts.output_format == CSV )
		fprintf(outf, "#,Time,Type,Value,Unit,\"Before meal\",\"After meal\",Stress,Sick,\"Dont feel right\",Activity,\"Control test\"\n");

	while (1) {
		ret = contour_read_entry(fd, usage_code, &msg);
		if (ret < 45)
			break;

		if ( opts.output_format == CSV ) {
			char *tok = (char *) &msg.data;
			              token(&tok, '|');  // unknown
			char *seq =   token(&tok, '|');
			char *type =  token(&tok, '|');
			char *val =   token(&tok, '|');
			char *unit =  token(&tok, '|');
			              token(&tok, '|');  // unknown
			char *notes = token(&tok, '|');
			              token(&tok, '|');  // unknown
			char *time =  token(&tok, '\r');
			unit[strlen(unit)-2] = 0;
			fprintf(outf, "%s,\"%.4s-%.2s-%.2s %.2s:%.2s\",\"%s\",\"%s\",\"%s\",%s,%s,%s,%s,%s,%s,%s\n",
				seq, &time[0], &time[4], &time[6], &time[8], &time[10], &type[3], val, unit,
				strchr(notes, 'B') ? "X" : "", strchr(notes, 'A') ? "X" : "",
				strchr(notes, 'S') ? "X" : "", strchr(notes, 'I') ? "X" : "",
				strchr(notes, 'D') ? "X" : "", strchr(notes, 'X') ? "X" : "",
				strchr(notes, 'C') ? "X" : ""
			);
		} else
		if ( opts.output_format == CLEAN ) {
			sanitize_ascii(msg.data, ret);
			fprintf(outf, "%s\n", msg.data);
		} else
		if ( opts.output_format == RAW ) {
			fprintf(outf, "%s", msg.data);
		}

		entries++;
		fflush(outf);

		if ((outf != stdout) || !isatty(fileno(stdout))) {
			trace(0, "\r%d entries", entries);
			fflush(stdout);
		}
	}
	trace(0, "\nDone.\n");

	return 0;
}

char *token(char **str, char sep)
{
  char *start = *str;
  char *cur;
  for ( cur = start; *cur && (*cur != sep); ++cur ) ;
  *cur = 0;
  *str = cur+1;
  return start;
}
