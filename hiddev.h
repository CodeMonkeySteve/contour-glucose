#ifndef __HIDDEV_H_
#define __HIDDEV_H_

#define READ_BUFFER_LENGTH 64

int hiddev_read(unsigned char *data, int bufsize, int fd);
int hiddev_write(const unsigned char data[64], int fd , int usage_code);
int hiddev_open(const char *device_path, int *usage_code);

#endif
