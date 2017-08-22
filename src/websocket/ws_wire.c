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

// Given a libevent evbuffer try and extract a websocket frame from the buffer.
// Note, we pass in a ws_nn_msghdr_pt to the decoder. Each frame is encoded into
// an nn_iovec. A single frame will be a single nn_iovec. However, if the websocket
// is sending a number of fragments in frag frames they are built up as an array
// of nn_iovec structures. See http://nanomsg.org/v1.0.0/nn_sendmsg.3.html
// Although we cannot steal the libevent iovec in it's evbuffer chains for zero-copy
// we can at least use NN's nn_iovec for fragmented messages. I wonder how many
// websocket fragments we get? ToDo: add metrics to track this.

#include <stdlib.h>

#include "ws_wire.h"

#ifdef __cplusplus
extern C {
#endif

typedef union
{
        unsigned char bytes[2];
        uint16_t len;
	unsigned char mask[4];
} 
ws_wire_len_126_t;

typedef union
{
        unsigned char bytes[2];
        uint64_t len;
	unsigned char mask[4];
} 
ws_wire_len_127_t;

typedef struct
{
	unsigned char control;
	unsigned char short_len;
	union {
		ws_wire_len_126_t for126;
		ws_wire_len_127_t for127;
	} extended;
}
ws_wire_header_t, *ws_wire_header_pt;

static uint64_t
payload_length(ws_wire_header_pt inp_header, int offset)
{
	switch(offset) {
	case 2: return (uint64_t)(inp_header->short_len & 0x7f);
	case 4: return (uint64_t)inp_header->extended.for126.len;
	case 10: return (uint64_t)inp_header->extended.for127.len;
	default: return (uint64_t)0;
	}
	return 0;
}


ws_nn_msghdr_pt
ws_wire_process(struct evbuffer *inp_evbuffer, ws_nn_msghdr_pt inp_nn_msghdr, int nn_flags)
{
	if(inp_evbuffer) {
		short short_len = 0;
		int mask = 0, offset = 0;
		ws_wire_header_t header;
		ev_ssize_t copied = evbuffer_copyout(inp_evbuffer, &header, sizeof(ws_wire_header_t));
		if(copied > 1) {
			short_len = header.short_len & 0x7fU;
			mask =  ((header.short_len & 0x80U) == 0x80U) ? 4 : 0;
			if(short_len < 126) {
				offset = 2;
			}
			else if(copied > 3 && short_len == 126) {
				offset = 4;
			}
			else if(copied > 9 && short_len == 127) {
				offset = 10;
			}
			else {
				return NULL;
			}
		}
		if(offset > 0 && copied >= (offset + mask)) {
			size_t evbuffer_len = evbuffer_get_length(inp_evbuffer);
			int64_t payload_len = payload_length(&header, offset);
			int64_t expected_len = payload_len + offset + mask; 
			if(payload_len > 0 && evbuffer_len >= expected_len) {
				// Prepare an NN Msg payload to carry the data forward.
				void *pv = NULL;
				int iov_index = 0;
				if((pv = nn_allocmsg(payload_len, nn_flags)) == NULL) {
					return NULL;
				}
				if(inp_nn_msghdr == NULL) {
					if((inp_nn_msghdr = ws_nn_msghdr_new(1)) == NULL) {
						nn_freemsg(pv);
						return NULL;
					}
				}
				iov_index = ws_nn_msghdr_vec_inc(inp_nn_msghdr); 
				inp_nn_msghdr->msghdr.msg_iov[iov_index].iov_base = pv;
				inp_nn_msghdr->msghdr.msg_iov[iov_index].iov_len = NN_MSG;	

				// Tracl the ws frame info in a seperate array. This allows
				// the caller to retrieve these items if required (like the opcode)
				inp_nn_msghdr->p_vecinfo[iov_index].fin = ((header.control & 0x80U) > 0) ? 1 : 0;
				inp_nn_msghdr->p_vecinfo[iov_index].opcode = header.control & 0x0fU;
				inp_nn_msghdr->p_vecinfo[iov_index].payload_len = payload_len;

				// Remove the websocket protocol header, no longer needed.
				evbuffer_drain(inp_evbuffer, offset + mask);

				// Copy (yes, copy) the evbuffer contents over to NN msg.
				// Both libevent evbuffers and NN messages use iovec struct
				// to manage their respective buffer sets. However, libevent
				// API doesn't offer a way to steal their reference and take
				// ownership to do a zero-copy operation. So (for now at least)
				// we must copy them over. Shame really, this is one place we
				// could probably get a perf boost and an inflight memory saving.
				evbuffer_remove(inp_evbuffer, pv, payload_len);	
				// If the data in the payload was masked then demask it now as
				// we do not retain the orginal 4 mask bytes,
				if(mask) {
					unsigned char *p = (unsigned char*)pv;
					unsigned char *p_mask = ((unsigned char*)&header) + offset;
					for(int64_t i = 0; i < payload_len; i++) {
						p[i] ^= p_mask[(i & 0x3)];
					}
				}
				// Finally, keep a copy of the control byte
				inp_nn_msghdr->total_size += payload_len;
			}
		}

	}
	return inp_nn_msghdr;
}

#ifdef __cplusplus
}
#endif

