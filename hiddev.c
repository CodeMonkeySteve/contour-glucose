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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <linux/hiddev.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>

#include "hiddev.h"
#include "utils.h"

#define PRINT_FIELD(level, field) trace(level, #field ": %04x\n", field)

#define HIDDEV_PATH "/dev/usb/"

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

static int _hiddev_open(const char *device_path, int *usage_code,
			int vendor_id, int product_id)
{
	struct hiddev_devinfo device_info;
	struct hiddev_report_info rinfo;
	int ret, error;
	int fd;

	trace(3, "Opening device %s\n", device_path);
	fd = ret = open(device_path, O_RDWR);

	if (fd < 0)
		goto err;

	rinfo.report_type = HID_REPORT_TYPE_OUTPUT;
	rinfo.report_id = HID_REPORT_ID_FIRST;
	ret = ioctl(fd, HIDIOCGREPORTINFO, &rinfo);
	if (ret < 0)
		goto err_close;

	PRINT_FIELD(3, rinfo.report_type);
	PRINT_FIELD(3, rinfo.report_id);
	PRINT_FIELD(3, rinfo.num_fields);

	*usage_code = get_usagecode(fd);

	if (*usage_code < 0)
		goto err_close;

	ret = ioctl(fd, HIDIOCGDEVINFO, &device_info);

	if (ret < 0)
		goto err_close;

	PRINT_FIELD(3, device_info.bustype);
	PRINT_FIELD(3, device_info.busnum);
	PRINT_FIELD(3, device_info.devnum);
	PRINT_FIELD(3, device_info.ifnum);
	PRINT_FIELD(3, device_info.vendor);
	PRINT_FIELD(3, device_info.product);
	PRINT_FIELD(3, device_info.version);
	PRINT_FIELD(3, device_info.num_applications);

	if (product_id && vendor_id) {
		if (product_id == device_info.product &&
			vendor_id == device_info.vendor)
			return fd;
		trace(3, "Vendor and product IDs don't match\n");
		ret = -1;
		goto err_close;
	}

	return fd;

err_close:
	close(fd);
err:
	error = errno;
	if (error)
		printf("Error opening device %s: %s\n", device_path,
			strerror(error));
	return ret;
}

int hiddev_open(const char *device_path, int *usage_code)
{
	return _hiddev_open(device_path, usage_code, 0, 0);
}

int hiddev_open_by_id(int vendor_id, int product_id, int *usage_code)
{
	struct dirent *dirent;
	DIR *dir;
	int error, fd;
	char path[256];

	dir = opendir(HIDDEV_PATH);
	if (dir == NULL) {
		error = errno;
		trace(4, "Failed to open directory %s: %s\n", HIDDEV_PATH,
			strerror(error));
		return -error;
	}

	while (1) {
		dirent = readdir(dir);
		if (dirent == NULL)
			break;

		if (strncmp(dirent->d_name, "hiddev", sizeof("hiddev") - 1))
			continue;

		path[0] = 0;
		strncat(path, HIDDEV_PATH, sizeof(path) - 1);
		strncat(path, dirent->d_name, sizeof(path) - 1);

		fd = _hiddev_open(path, usage_code, vendor_id, product_id);
		if (fd < 0)
			continue;
		return fd;
	}

	if (errno) {
		error = errno;
		trace(0, "Error reading directory %s: %s\n", HIDDEV_PATH,
			strerror(error));
		return -error;
	}

	trace(0, "Canno't find any mathing hiddev devices\n");
	return -1;
}
