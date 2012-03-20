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

#ifndef _UTILS_H
#define _UTILS_H

#include <stdio.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static inline int is_printable(const unsigned char c)
{
	return c >= 0x20 && c < 0x80;
}

extern int trace_level;

#define trace(level, arg...)			\
	do {					\
		if (level <= trace_level) 	\
			fprintf(stderr, arg);	\
	} while (0)

int datalen(const unsigned char *data);
void print_hex(const unsigned char *data, int len);
void print_ascii(const unsigned char *data, int len);
void sanitize_ascii(unsigned char *data, int len);

#endif
