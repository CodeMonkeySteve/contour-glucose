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
		if (level <= trace_level)	\
			printf(arg);		\
	} while (0)

int datalen(const unsigned char *data);
void print_hex(const unsigned char *data, int len);
void print_ascii(const unsigned char *data, int len);

#endif
