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
