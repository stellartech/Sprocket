

#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>

#define WS_FRAME_FRIEND
#include "ws_frame.h"

ws_frame_pt
ws_frame_ctor(void)
{
	ws_frame_pt p_self = calloc(1, sizeof(ws_frame_t));
	if(p_self) {
	}
	return p_self;
}

void
ws_frame_free(ws_frame_pt inp_self)
{
	if(inp_self) {
		inp_self->refcount--;
		if(inp_self->refcount == 0) {
			inp_self->frame_in = 0;
			if(inp_self->p_frame) {
				free(inp_self->p_frame);
				inp_self->p_frame = NULL;
			}
			inp_self->payload_len = 0;
			free(inp_self);
		}
	}
}

void
ws_frame_dtor(ws_frame_pt *inpp_self)
{
	if(inpp_self) {
		int refcount_copy = 0;
		ws_frame_pt p_self = *inpp_self;
		if(p_self) {
			refcount_copy = p_self->refcount;
			ws_frame_free(p_self);
		}
		refcount_copy--;
		if(refcount_copy == 0) {
			*inpp_self = NULL;
		}
	}
}

uint64_t
ws_frame_append_chunk(ws_frame_pt inp_self, char *inp, uint64_t in_len)
{
	uint64_t new_len = 0;
	if(inp_self && inp && in_len > 0) {
		new_len = inp_self->frame_in + in_len;
		if(!inp_self->p_frame) {
			inp_self->p_frame = calloc(1, new_len);
		}
		else {
			inp_self->p_frame = realloc(inp_self->p_frame, new_len);
		}
		if(!inp_self->p_frame) {
			// ToDo
		}
		else {
			memcpy(inp_self->p_frame + inp_self->frame_in, inp, in_len); 
			inp_self->frame_in += in_len;
		}
	}
	return new_len;
}

uint64_t
ws_frame_append_bufferevent(ws_frame_pt inp_self, struct bufferevent *inp_bev)
{
	int len;
	struct evbuffer *p_evbuf_in = bufferevent_get_input(inp_bev);
	if((len = evbuffer_get_length(p_evbuf_in)) > 0) {
		uint64_t new_len = inp_self->frame_in + len;
		if(!inp_self->p_frame) {
			inp_self->p_frame = calloc(1, new_len);
		}
		else {
			inp_self->p_frame = realloc(inp_self->p_frame, new_len);
		}
		if(inp_self->p_frame) {
			int len_read = 0;
			while(len_read < len) {
				int i = 0;
				char *p = inp_self->p_frame;
				p += inp_self->frame_in;
				i = bufferevent_read(inp_bev, p, len - len_read);
				len_read += i;
				inp_self->frame_in += i;
			}
		}
		return new_len;
	}
	return 0;
}

// Forward local prototypes.
static int ws_frame_test_125(ws_frame_pt inp_self, char *in_mask);
static int ws_frame_test_126(ws_frame_pt inp_self, char *in_mask);
static int ws_frame_test_127(ws_frame_pt inp_self, char *in_mask);

static inline int
ws_frame_is_masked(const char* inp_packet)
{
    return ((unsigned char)inp_packet[0]) & 0x80 ? 1 : 0;
}

int
ws_frame_is_valid(ws_frame_pt inp_self)
{
	char a_mask[4] = {0,0,0,0};
	unsigned char short_len = 0;
	char* p_frame = inp_self->p_frame;
   
	inp_self->p_payload = NULL;
   
	// At a minimum we must have two bytes
	// in the input buffer to proceed.
	if(inp_self->frame_in < 2) {
		return 0;
	}
   
	inp_self->payload_len = 0;
	inp_self->p_payload = NULL;

	short_len = (uint64_t)(inp_self->p_frame[1] & 0x7f);

	if(short_len <= 125) {
		if(0 == ws_frame_test_125(inp_self, a_mask)) {
			return 0;
		}
	}
	else if(short_len == 126) {
		if(0 == ws_frame_test_126(inp_self, a_mask)) {
			return 0;
		}
	}
	else if(short_len == 127) {
		if(0 == ws_frame_test_127(inp_self, a_mask)) {
			return 0;
		}
	}

	if(ws_frame_is_masked(p_frame)) {
		for(int i = 0; i < inp_self->payload_len; i++) {
			inp_self->p_payload[i] ^= a_mask[(i & 0x3)];
		}
	}
   
	return 1;
}

static int
ws_frame_test_125(ws_frame_pt inp_self, char *in_mask)
{
	int offset = ws_frame_is_masked(inp_self->p_frame) ? 6 : 2;
	uint64_t len = (uint64_t)(inp_self->p_frame[1] & 0x7f);
	if(inp_self->frame_in >= len + offset) {
		if(offset == 6) {
			in_mask[0] = inp_self->p_frame[2];
			in_mask[1] = inp_self->p_frame[3];
			in_mask[2] = inp_self->p_frame[4];
			in_mask[3] = inp_self->p_frame[5];
			inp_self->p_payload = &inp_self->p_frame[6];
		}
		else {
			inp_self->p_payload = &inp_self->p_frame[2];
		}
		inp_self->payload_len = len;
		return 1;
	}
	return 0;
}

static int
ws_frame_test_126(ws_frame_pt inp_self, char *in_mask)
{
	uint64_t len = 0;
	int offset = ws_frame_is_masked(inp_self->p_frame) ? 8 : 4;
	if(inp_self->frame_in < 4) {
		return 0;
	}
	len = ((unsigned char)inp_self->p_frame[2] << 8) + (unsigned char)inp_self->p_frame[3];
	if(inp_self->frame_in >= len + offset) {
		if(offset == 8) {
			in_mask[0] = inp_self->p_frame[4];
			in_mask[1] = inp_self->p_frame[5];
			in_mask[2] = inp_self->p_frame[6];
			in_mask[3] = inp_self->p_frame[7];
			inp_self->p_payload = &inp_self->p_frame[8];
		}
		else {
			inp_self->p_payload = &inp_self->p_frame[4];
		}
		inp_self->payload_len = len;
		return 1;
	}
	return 0;
}

static int
ws_frame_test_127(ws_frame_pt inp_self, char *in_mask)
{
	uint64_t len = 0, h = 0, l = 0;
	char *p_frame = inp_self->p_frame;
	int offset = ws_frame_is_masked(inp_self->p_frame) ? 14 : 10;
	if(inp_self->frame_in < 10) {
		return 0;
	}
	h = (p_frame[2] << 24) + (p_frame[3] << 16) + 
		(p_frame[4] << 8)  + (p_frame[5] << 0);
	l = (p_frame[6] << 24) + (p_frame[7] << 16) + 
		(p_frame[8] << 8)  + (p_frame[9] << 0);
	len = (h << 32) + l;
	if(inp_self->frame_in >= len + offset) {
		if(offset == 14) {
			in_mask[0] = p_frame[10];
			in_mask[1] = p_frame[11];
			in_mask[2] = p_frame[12];
			in_mask[3] = p_frame[13];
			inp_self->p_payload = &inp_self->p_frame[14];
		}
		else {
			inp_self->p_payload = &inp_self->p_frame[10];
		}
		inp_self->payload_len = len;
		return 1;
	}
	return 0;
}

