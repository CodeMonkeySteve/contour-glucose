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

#include "utils.h"

int trace_level;

int datalen(const unsigned char *data)
{
	int i, len;

	for (i = 0, len = 0; i < 64; i++)
		if (data[i])
			len = i;

	return len + 1;
}

void print_hex(const unsigned char *data, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("0x%02x ", data[i]);

	printf("\n");
}

void print_ascii(const unsigned char *data, int len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%c", is_printable(data[i]) ? data[i] : '.');

	printf("\n");
}

void sanitize_ascii(unsigned char *data, int len)
{
	int i;

	for (i = 0; i < len; i++)
		data[i] = is_printable(data[i]) ? data[i] : '.';
	data[i] = 0;
}
