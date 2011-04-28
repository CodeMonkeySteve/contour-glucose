#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/hiddev.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "hiddev.h"
#include "utils.h"


struct msg {
	int direction;
	unsigned char data[64];
};

enum direction {
	IN = 666,
	OUT,
};

int send_msg(const struct msg *msg, int fd, int usage_code)
{
	int ret;

	if (msg->direction != OUT) {
		trace(0, "Message direction is not OUT\n");
		exit(1);
	}

	usleep(30 * 1000);
	trace(1, "Sending: ");
	if (trace_level >= 3)
		print_hex(msg->data + 1, datalen(msg->data) - 1);
	if (trace_level >= 2)
		print_ascii(msg->data + 1, datalen(msg->data) - 1);

	ret = hiddev_write(msg->data, fd, usage_code);
	if (ret)
		exit(1);

	return 0;
}

int read_and_verify(struct msg *msg, int fd)
{
	unsigned char buf[64];
	int ret, offset = 0;

	while (offset < 64) {
		ret = hiddev_read(buf + offset, sizeof(buf) - offset, fd);

		if (ret < 0)
			goto err;

		offset += ret;
	}

	memcpy(msg->data, buf, sizeof(buf));
	trace(2, "Got data %d: ", datalen(buf));
	if (trace_level >= 3)
		print_hex(buf, datalen(buf));
	if (trace_level >= 2)
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

	usleep(100 * 1000);
	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(250 * 1000);
	SET_BYTE(1, 0x15);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x05);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	SET_FIRST_BYTE(0x02);
	SET_BYTES(byte1);
	SET_BYTES('|');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
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
		usleep(100 * 1000);
		send_msg(&msg, fd, uc);
		read_msgs(fd);
	}
	usleep(1000 * 1000);
	read_msgs(fd);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
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

	usleep(100 * 1000);
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

	usleep(410 * 1000);
	SET_FIRST_BYTE(0x02);
	SET_BYTES('R');
	SET_BYTES('|');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'Y');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'W');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'K');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'C');
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	send_pattern(fd, uc, 'R', 'Z');

	usleep(100 * 1000);
	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x15);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x05);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x04);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x06);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	send_msg(&msg, fd, uc);
	read_msgs(fd);

	trace(0, "Glucose readings:\n");
	usleep(100 * 1000);
	do {
		send_msg(&msg, fd, uc);
		read_and_verify(&in, fd);
		print_ascii(in.data, datalen(in.data));
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
