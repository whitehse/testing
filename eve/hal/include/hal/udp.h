
/*
 * Copyright (C) 2014 Dan White
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef HAL_UDP_H
#define HAL_UDP_H

/* Return types. But only use these internally! */
/* Use, for example, HALOK, when returning from public
 * functions. */
#define HAL_UDP_OK               0 /* successful result */
#define HAL_UDP_FAIL            -1 /* generic failure */
#define HAL_UDP_NOMEM           -2 /* memory shortage failure */
#define HAL_UDP_BUFOVER         -3 /* overflowed buffer */
#define HAL_UDP_BADPARAM        -4 /* invalid parameter supplied */
#define HAL_UDP_BAD_DATA        -5 /* Invalid Packet */
#define HAL_UDP_NA              -6 /* Invalid Packet */

#define HAL_UDP_MIN_SEGMENT_SIZE 5
#define HAL_UDP_MAX_SEGMENT_SIZE 65535

struct hal_udp {
    uint16_t source_port;	// Host order
    uint16_t destination_port;  // Host order
    uint16_t payload_length;    // Host order
    uint16_t checksum;          // Host order
    char* payload_pointer;
};

#endif // HAL_UDP_H
