/*
 * The Qubes OS Project, http://www.qubes-os.org
 *
 * Copyright (C) 2010  Rafal Wojtczuk  <rafal@invisiblethingslab.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _QUBES_TXRX_H
#define _QUBES_TXRX_H

#include <libvchan.h>

void write_data(libvchan_t *vchan, void *buf, size_t size);
void real_write_message(libvchan_t *vchan, char *hdr, size_t size, char *data, size_t datasize);
size_t read_data(libvchan_t *vchan, char *buf, size_t size);
#define read_struct(vchan, x) read_data(vchan, (char*)&x, sizeof(x))
#define write_struct(vchan, x) write_data(vchan, (char*)&x, sizeof(x))
#define write_message(vchan,x,y) do {\
    x.untrusted_len = sizeof(y); \
    real_write_message(vchan, (char*)&x, sizeof(x), (char*)&y, sizeof(y)); \
    } while(0)
int wait_for_vchan_or_argfd(libvchan_t *vchan, int fd);
void vchan_register_at_eof(void (*new_vchan_at_eof)(void));

#endif /* _QUBES_TXRX_H */
