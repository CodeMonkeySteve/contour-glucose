#ifndef _CONTOUR_USB_H
#define _CONTOUR_USB_H

#define CONTOUR_USB_VENDOR_ID	0x1a79
#define CONTOUR_USB_PRODUCT_ID	0x6002

struct msg {
	int direction;
	unsigned char data[64];
};

enum direction {
	IN = 666,
	OUT,
};

int contour_initialize(int fd, int uc);
int contour_read_entry(int fd, int uc, struct msg *in);
int wait_for_device(int vendor, int product, int *usage_code);


#endif
