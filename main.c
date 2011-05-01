#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <string.h>

#include "hiddev.h"
#include "utils.h"
#include "options.h"
#include "contour-protocol.h"

int main(int argc, char *argv[])
{
	int fd, usage_code, ret;
	struct user_options opts;
	struct msg msg;

	read_args(argc, argv, &opts);
	trace_level = opts.trace_level;

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
		print_ascii(msg.data, ret);

		if (ret < 45)
			break;
	}

	return 0;
}
