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

#ifndef __HIDDEV_H_
#define __HIDDEV_H_

#define READ_BUFFER_LENGTH 64

int hiddev_read(unsigned char *data, int bufsize, int fd);
int hiddev_write(const unsigned char data[64], int fd , int usage_code);
int hiddev_open(const char *device_path, int *usage_code);
int hiddev_open_by_id(int vendor_id, int product_id, int *usage_code);

#endif
