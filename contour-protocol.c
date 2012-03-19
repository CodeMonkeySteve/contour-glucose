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

#include <stdlib.h>
#include <unistd.h>
#include <linux/types.h>
#include <linux/hiddev.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "hiddev.h"
#include "contour-protocol.h"
#include "utils.h"

#define MAX_MSGS	103

static int send_msg(const struct msg *msg, int fd, int usage_code)
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

static int send_msg_with_proggress_note(const struct msg *msg, int fd,
					int usage_code)
{
	static int msg_count;

	if (trace_level < 1 && msg_count <= MAX_MSGS) {
		trace(0, "\r%d%%", msg_count * 100 / MAX_MSGS);
		fflush(stdout);
	}

	msg_count++;

	return send_msg(msg, fd, usage_code);
}

static int read_and_verify(struct msg *msg, int fd)
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

static int read_msgs(int fd)
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

static int send_pattern(int fd, int uc, unsigned char byte1, unsigned char byte2)
{
	int j;
	struct msg msg;
	msg.direction = OUT;

	usleep(100 * 1000);
	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(250 * 1000);
	SET_BYTE(1, 0x15);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x05);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	SET_FIRST_BYTE(0x02);
	SET_BYTES(byte1);
	SET_BYTES('|');
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, byte2);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	return 0;
}

int contour_initialize(int fd, int uc)
{
	int i, j;
	struct msg msg;
	msg.direction = OUT;

	read_msgs(fd);
	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg_with_proggress_note(&msg, fd, uc);

	usleep(100 * 1000);
	SET_BYTE(1, 0x06);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	SET_BYTE(1, 0x15);
	for (i = 0; i < 6; i++) {
		usleep(100 * 1000);
		send_msg_with_proggress_note(&msg, fd, uc);
		read_msgs(fd);
	}
	usleep(1000 * 1000);
	read_msgs(fd);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x05);
	send_msg_with_proggress_note(&msg, fd, uc);
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
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(410 * 1000);
	SET_FIRST_BYTE(0x02);
	SET_BYTES('R');
	SET_BYTES('|');
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'Y');
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'W');
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'K');
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 'C');
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	send_pattern(fd, uc, 'R', 'Z');

	usleep(100 * 1000);
	SET_FIRST_BYTE(0x01);
	SET_BYTES(0x04);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x15);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x05);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x04);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	SET_BYTE(1, 0x06);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);
	send_msg_with_proggress_note(&msg, fd, uc);
	read_msgs(fd);

	usleep(100 * 1000);

	trace(0, "\n");
	return 0;
}

int contour_read_entry(int fd, int uc, struct msg *in)
{
	struct msg msg;
	int j;

	msg.direction = OUT;
	SET_FIRST_BYTE(0x01);
	SET_BYTE(1, 0x06);

	send_msg(&msg, fd, uc);
	read_and_verify(in, fd);

	return datalen(in->data);
}

int wait_for_device(int vendor, int product, int *usage_code)
{
	int fd;

	fd = hiddev_open_by_id(vendor, product, usage_code);

	if (fd > 0)
		return fd;

	trace(0,
	       "No suitable device found. Please plug in your glucose meter\n");
	do {
		usleep(500 * 1000);
		fd = hiddev_open_by_id(vendor, product, usage_code);
	} while(fd < 0);

	usleep(2000 * 1000);

	return fd;
}

