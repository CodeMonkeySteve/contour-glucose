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
	int fd, usage_code;
	struct user_options opts;

	read_args(argc, argv, &opts);
	trace_level = opts.trace_level;

	if (opts.usbdev == NULL)
		fd = wait_for_device(CONTOUR_USB_VENDOR_ID,
				CONTOUR_USB_PRODUCT_ID, &usage_code);
	else
		fd = hiddev_open(opts.usbdev, &usage_code);
	if (fd < 0)
		return 1;

	communicate(fd, usage_code);

	return 0;
}
