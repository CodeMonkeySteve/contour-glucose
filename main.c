#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/hiddev.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define PRINT_FIELD(field) printf(#field ": %04x\n", field)

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#define READ_BUFFER_LENGTH 64

struct msg {
	int direction;
	unsigned char data[64];
};

enum direction {
	IN = 666,
	OUT,
};

static int is_printable(const unsigned char c)
{
	return c >= 0x20 && c < 0x80;
}

static int datalen(const unsigned char *data)
{
	int i, len;

	for (i = 0, len = 0; i < 64; i++)
		if (data[i])
			len = i;

	return len + 1;
}

static void print_hex(const unsigned char *data, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("0x%02x ", data[i]);

	printf("\n");
}

static void print_ascii(const unsigned char *data, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%c", is_printable(data[i]) ? data[i] : '.');

	printf("\n");
}

int hid_read(unsigned char *data, int bufsize, int fd)
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


int hid_write(const unsigned char data[64], int fd , int usage_code)
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

int get_usagecode(int fd)
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

	PRINT_FIELD(rinfo.report_type);
	PRINT_FIELD(rinfo.report_id);
	PRINT_FIELD(rinfo.num_fields);

	*usage_code = get_usagecode(fd);

	if (*usage_code < 0)
		return -8;

	ret = ioctl(fd, HIDIOCGDEVINFO, &device_info);

	if (ret < 0)
		goto err;

	PRINT_FIELD(device_info.bustype);
	PRINT_FIELD(device_info.busnum);
	PRINT_FIELD(device_info.devnum);
	PRINT_FIELD(device_info.ifnum);
	PRINT_FIELD(device_info.vendor);
	PRINT_FIELD(device_info.product);
	PRINT_FIELD(device_info.version);
	PRINT_FIELD(device_info.num_applications);

	return fd;

err:
	error = errno;
	printf("Error opening device %s: %s\n", device_path, strerror(error));
	return ret;
}

int send_msg(const struct msg *msg, int fd, int usage_code)
{
	int ret;

	if (msg->direction != OUT) {
		printf("Message direction is not OUT\n");
		exit(1);
	}

	usleep(1000 * 1000);
	printf("Sending: ");
	print_hex(msg->data + 1, datalen(msg->data) - 1);
	print_ascii(msg->data + 1, datalen(msg->data) - 1);

	ret = hid_write(msg->data, fd, usage_code);
	if (ret)
		exit(1);

	return 0;
}

int read_and_verify(struct msg *msg, int fd)
{
	unsigned char buf[64];
	int ret, offset = 0;

	while (offset < 64) {
		ret = hid_read(buf + offset, sizeof(buf) - offset, fd);

		if (ret < 0)
			goto err;

		offset += ret;
	}

	memcpy(msg->data, buf, sizeof(buf));
	printf("Got data %d: ", datalen(buf));
//	print_hex(buf, datalen(buf));
	print_ascii(buf, datalen(buf));
err:
	return 0;
}

int read_msgs(int fd)
{
	struct msg msg;
	while (1) {
		read_and_verify(&msg, fd);
		if (datalen(msg.data) <= 36)
			break;
	}

	return 0;
}

#define SET_FIRST_BYTE(byte)					\
	do {							\
		memset(&msg.data, 0, sizeof(msg.data));		\
		msg.data[4] = (byte);				\
		j = 5;						\
	} while (0)

#define SET_BYTE(idx, byte)			\
	do {					\
		msg.data[(idx) + 4] = (byte);	\
		j = (idx) + 1;			\
	} while (0)

#define SET_BYTES(byte) msg.data[j++] = (byte)

int send_pattern(int fd, int uc, unsigned char byte1, unsigned char byte2)
{
	int j;
	struct msg msg;
	msg.direction = OUT;

	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x15);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x05);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_FIRST_BYTE(0x02);
	SET_BYTES(byte1);
	SET_BYTES('|');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, byte2);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	return 0;
}

int communicate(int fd, int uc)
{
	int i, j;
	struct msg msg, in;
	msg.direction = OUT;

	read_msgs(fd);
	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg(&msg, fd, uc);

	usleep(100 * 1000);
	SET_BYTE(1, 0x06);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x15);
	for (i = 0; i < 6; i++) {
		send_msg(&msg, fd, uc);
		read_msgs(fd);
	}
	usleep(1000 * 1000);
	read_msgs(fd);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x05);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	send_pattern(fd, uc, 'R', 'A');
	send_pattern(fd, uc, 'R', 'C');
	send_pattern(fd, uc, 'R', 'D');
	send_pattern(fd, uc, 'R', 'G');
	send_pattern(fd, uc, 'R', 'I');
	send_pattern(fd, uc, 'R', 'M');
	send_pattern(fd, uc, 'R', 'P');
	send_pattern(fd, uc, 'R', 'R');
	send_pattern(fd, uc, 'R', 'S');
	send_pattern(fd, uc, 'R', 'T');
	send_pattern(fd, uc, 'R', 'U');
	send_pattern(fd, uc, 'R', 'V');
	send_pattern(fd, uc, 'R', 'W');
	send_pattern(fd, uc, 'R', 'X');
	send_pattern(fd, uc, 'W', 'K');

	SET_FIRST_BYTE(0x08);
	SET_BYTES('O');
	SET_BYTES('b');
	SET_BYTES('p');
	SET_BYTES('s');
	SET_BYTES('e');
	SET_BYTES('c');
	SET_BYTES('3');
	SET_BYTES('|');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_FIRST_BYTE(0x02);
	SET_BYTES('R');
	SET_BYTES('|');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 'Y');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 'W');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 'K');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 'C');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	send_pattern(fd, uc, 'R', 'Z');

	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x15);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x05);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x04);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x06);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	send_msg(&msg, fd, uc);
	read_msgs(fd);

	send_msg(&msg, fd, uc);
	read_msgs(fd);

	do {
		send_msg(&msg, fd, uc);
		read_and_verify(&in, fd);
	} while (datalen(in.data) > 45);

	return 0;
}

int main(int argc, char *argv[])
{
	int fd, usage_code;

	fd = hiddev_open(argv[1], &usage_code);
	if (fd < 0)
		return 1;

	communicate(fd, usage_code);

	return 0;
}
