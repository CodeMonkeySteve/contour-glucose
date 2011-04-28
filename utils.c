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
