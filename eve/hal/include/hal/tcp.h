/*
 * Copyright (C) 2023 Dan White
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

#ifndef HAL_TCP_H
#define HAL_TCP_H

/* Return types. But only use these internally! */
/* Use, for example, HALOK, when returning from public
 * functions. */
#define HAL_TCP_OK                0 /* successful result */
#define HAL_TCP_FAIL             -1 /* generic failure */
#define HAL_TCP_NOMEM            -2 /* memory shortage failure */
#define HAL_TCP_BUFOVER          -3 /* overflowed buffer */
#define HAL_TCP_BADPARAM         -4 /* invalid parameter supplied */
#define HAL_TCP_BAD_DATA         -5 /* Invalid Packet */
#define HAL_TCP_NA               -6 /* Invalid Packet */

#define HAL_TCP_FLAG_FIN         0x001
#define HAL_TCP_FLAG_SYN         0x002
#define HAL_TCP_FLAG_RST         0x004
#define HAL_TCP_FLAG_PSH         0x008
#define HAL_TCP_FLAG_ACK         0x010
#define HAL_TCP_FLAG_URG         0x020
#define HAL_TCP_FLAG_ECE         0x040
#define HAL_TCP_FLAG_CWR         0x080
#define HAL_TCP_FLAG_NS          0x100

#define HAL_TCP_MIN_SEGMENT_SIZE 20
#define HAL_TCP_MAX_SEGMENT_SIZE 65535

struct hal_tcp {
    uint16_t source_port;	// Host order
    uint16_t destination_port;
    uint32_t seq_number;
    uint32_t ack_number;
    uint16_t offset;
    uint16_t flags;
    uint16_t window_size;
    uint16_t checksum;
    uint16_t urgent_pointer;
    uint16_t payload_length;
    char* payload_pointer;
    // Options
    // 1 for exists. 0 if it isn't in the header options
    uint8_t maximum_segment_size_exists;
    uint16_t maximum_segment_size;
    uint8_t window_scale_exists;
    uint8_t window_scale;
    // This is a flag used in the tcp setup. There is no data
    uint8_t sack_allowed_exists;
    // There can be up to 4 sack blocks. valid values are 0-4
    uint8_t sack_block_count;
    uint16_t sack_block_1;
    uint16_t sack_block_2;
    uint16_t sack_block_3;
    uint16_t sack_block_4;
    uint8_t timestamp_exists;
    uint16_t timestamp_value;
    uint16_t timestamp_echo_reply;
};

#endif // HAL_TCP_H
