/*********************************************************************************
 *   Copyright (c) 2008-2017 Andy Kirkham  All rights reserved.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included
 *   in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 ***********************************************************************************/

#ifndef WS_FRAG_MAKER_H_INCLUDED
#define WS_FRAG_MAKER_H_INCLUDED

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define WS_FRAG_FRIEND
#include "websocket/ws_frag.h"

static uint64_t 
load_random(uint64_t in_how_much, unsigned char *inp_dst)
{
	uint64_t randomDataRead = 0;
	int randomFd = open("/dev/urandom", O_RDONLY);
	if(randomFd) {
		randomDataRead = read(randomFd, inp_dst, in_how_much);
		close(randomFd);	
	}
	return randomDataRead;
}

static unsigned char*
ws_make_test_buffer(
	uint64_t test_size, 
	unsigned char control, 
	unsigned char *p_mask,
	int *outp_offset)
{
	int offset, mask = 0;
	uint64_t copied;
	unsigned char *p_test_data;
	//test_size = test_items[_i].test_size;
	mask = (p_mask[0]!=0||p_mask[1]!=0||p_mask[2]!=0||p_mask[3]!=0) ?  4 : 0;
	p_test_data = calloc(1, test_size + 256);
	ws_frag_pt p_local = ws_frag_ctor();
	p_test_data[0] = control; //0x83;
	if(test_size < 126) {
		offset = 2 + mask;
		p_test_data[1] = (unsigned char)(test_size & 0x7F);
		if(mask) {
			p_test_data[2] = p_mask[0];
			p_test_data[3] = p_mask[1];
			p_test_data[4] = p_mask[2];
			p_test_data[5] = p_mask[3];
		}
		load_random(test_size, &p_test_data[offset]);
	}
	else if(test_size >= 126 && test_size < 65536) {
		offset = 4 + mask;
		p_test_data[1] = 126;
		p_test_data[2] = (unsigned char)(test_size >> 8) & 0xFF;
		p_test_data[3] = (unsigned char)test_size & 0xFF;
		if(mask) {
			p_test_data[4] = p_mask[0];
			p_test_data[5] = p_mask[1];
			p_test_data[6] = p_mask[2];
			p_test_data[7] = p_mask[3];
		}
		load_random(test_size, &p_test_data[offset]);
	}
	else {
		offset = 10 + mask;
		p_test_data[1] = 127;
		p_test_data[2] = (unsigned char)((test_size >> 56) & 0xFF);
		p_test_data[3] = (unsigned char)((test_size >> 48) & 0xFF);
		p_test_data[4] = (unsigned char)((test_size >> 40) & 0xFF);
		p_test_data[5] = (unsigned char)((test_size >> 32) & 0xFF);
		p_test_data[6] = (unsigned char)((test_size >> 24) & 0xFF);
		p_test_data[7] = (unsigned char)((test_size >> 16) & 0xFF);
		p_test_data[8] = (unsigned char)((test_size >> 8) & 0xFF);
		p_test_data[9] = (unsigned char)(test_size & 0xFF);
		if(mask) {
			p_test_data[10] = p_mask[0];
			p_test_data[11] = p_mask[1];
			p_test_data[12] = p_mask[2];
			p_test_data[13] = p_mask[3];
		}
		load_random(test_size, &p_test_data[offset]);
	}
	if(mask) p_test_data[1] |= 0x80;
	*outp_offset = offset;
	return p_test_data;
}

#endif /* WS_FRAG_MAKER_H_INCLUDED */

