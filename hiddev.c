#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/hiddev.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include "hiddev.h"
#include "utils.h"

#define PRINT_FIELD(level, field) trace(level, #field ": %04x\n", field)

int hiddev_read(unsigned char *data, int bufsize, int fd)
{
	struct hiddev_event readBuffer[READ_BUFFER_LENGTH];
	int ret, n, i;

	ret = read(fd, readBuffer, sizeof(readBuffer));
	if (ret < 0)
		return ret;

	n = ret / sizeof(readBuffer[0]);
	for (i = 0; i < n && i < bufsize; i++)
		data[i] = readBuffer[i].value;
	return n;
}

int hiddev_write(const unsigned char data[64], int fd , int usage_code)
{
	int rc = 0, uindex, error;

	struct hiddev_usage_ref uref;
	struct hiddev_report_info rinfo;

	uref.report_id = *data++;
	uref.report_type = HID_REPORT_TYPE_OUTPUT;
	uref.field_index = 0;

	uref.usage_code = usage_code;

	for (uindex = 0; uindex < 63; uindex++) {
		uref.usage_index = uindex;
		uref.value = *data++;

		rc = ioctl(fd, HIDIOCSUSAGE, &uref);
		if (rc != 0)
			goto err;
	}

	rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id =  0x0;
	rinfo.num_fields = 1;

	rc = ioctl(fd, HIDIOCSREPORT, &rinfo);
	if (rc != 0)
		goto err2;

	return 0;
err2:
	printf("HIDIOCSREPORT\n");
err:
	error = errno;
	printf("Error in IOCTL: %s\n", strerror(error));

	return rc;
}

static int get_usagecode(int fd)
{
	struct hiddev_usage_ref uref;
	int rc, error;

	uref.report_type = HID_REPORT_TYPE_OUTPUT;
	uref.report_id = 0x0;
	uref.field_index = 0;
	uref.usage_index = 0;

	rc = ioctl(fd, HIDIOCGUCODE, &uref);
	if (rc < 0) {
		error = errno;
		printf("Error gettin usage code: %s\n", strerror(error));
		return rc;
	}

	return uref.usage_code;
}

int hiddev_open(const char *device_path, int *usage_code)
{
	struct hiddev_devinfo device_info;
	struct hiddev_report_info rinfo;
	int ret, error;
	int fd;
	fd = ret = open(device_path, O_RDWR);

	if (fd < 0)
		goto err;


	rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id = HID_REPORT_ID_FIRST;
	ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
	if (ret < 0)
		goto err;

	PRINT_FIELD(3, rinfo.report_type);
	PRINT_FIELD(3, rinfo.report_id);
	PRINT_FIELD(3, rinfo.num_fields);

	*usage_code = get_usagecode(fd);

	if (*usage_code < 0)
		return -8;

	ret = ioctl(fd, HIDIOCGDEVINFO, &device_info);

	if (ret < 0)
		goto err;

	PRINT_FIELD(3, device_info.bustype);
	PRINT_FIELD(3, device_info.busnum);
	PRINT_FIELD(3, device_info.devnum);
	PRINT_FIELD(3, device_info.ifnum);
	PRINT_FIELD(3, device_info.vendor);
	PRINT_FIELD(3, device_info.product);
	PRINT_FIELD(3, device_info.version);
	PRINT_FIELD(3, device_info.num_applications);

	return fd;

err:
	error = errno;
	printf("Error opening device %s: %s\n", device_path, strerror(error));
	return ret;
}

