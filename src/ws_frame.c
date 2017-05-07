

#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>

#include "thd.h"

#define WS_FRAME_FRIEND
#include "ws_frame.h"

ws_frame_pt
ws_frame_ctor(void)
{
	ws_frame_pt p_self = calloc(1, sizeof(ws_frame_t));
	if(p_self) {
		THREAD_INIT(&p_self->lock, NULL);
	}
	return p_self;
}

void
ws_frame_free(ws_frame_pt inp_self)
{
	if(inp_self) {
		THREAD_LOCK(&inp_self->lock);
		inp_self->refcount--;
		if(inp_self->refcount == 0) {
			inp_self->frame_in = 0;
			if(inp_self->p_frame) {
				free(inp_self->p_frame);
				inp_self->p_frame = NULL;
			}
			inp_self->payload_len = 0;
			THREAD_UNLOCK(&inp_self->lock);
			THREAD_LOCK_DESTROY(&inp_self->lock);
			free(inp_self);
		}
		else {
			THREAD_UNLOCK(&inp_self->lock);
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
ws_frame_append_chunk(ws_frame_pt inp_self, char *inp, int in_len)
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

static inline int
ws_frame_is_masked(const char* inp_packet)
{
    return ((unsigned char)inp_packet[0]) & 0x80 ? 1 : 0;
}

int
ws_frame_is_valid(ws_frame_pt inp_self)
{
	uint64_t len = 0;
	char  a_mask[4];
	char* p_frame = inp_self->p_frame;
   
	inp_self->p_payload = NULL;
   
	if(inp_self->frame_in < 2) {
		return 0;
	}
   
	len = (uint64_t)(inp_self->p_frame[1] & 0x7f);
   
	if(len < inp_self->payload_len) {
        	/* Not yet read the full frame. */
		return 0;
	}
   
	inp_self->payload_len = len;
   
	memset(a_mask, 0, sizeof(a_mask));
   
	if(len == 126) {
		len = (p_frame[2] << 8) + p_frame[3];
		if(ws_frame_is_masked(p_frame)) {
			a_mask[0] = p_frame[4];
			a_mask[1] = p_frame[5];
			a_mask[2] = p_frame[6];
			a_mask[3] = p_frame[7];
			inp_self->p_payload = &p_frame[8];
		}
		else {
			inp_self->p_payload = &p_frame[4];
		}
	}
	else if(len <= 125) {
		if(ws_frame_is_masked(p_frame)) {
			a_mask[0] = p_frame[2];
			a_mask[1] = p_frame[3];
			a_mask[2] = p_frame[4];
			a_mask[3] = p_frame[5];
			inp_self->p_payload = &p_frame[6];
		}
		else {
			inp_self->p_payload = &p_frame[2];
		}
	}
	else { /* == 127 */
		uint64_t h, l;
		h = (p_frame[2] << 24) | (p_frame[3] << 16) |
			(p_frame[4] << 8)  | (p_frame[5] << 0);
		l = (p_frame[6] << 24) | (p_frame[7] << 16) |
			(p_frame[8] << 8)  | (p_frame[9] << 0);
		len = (h << 32) | l;
		if(len < inp_self->frame_in) {
			/* Not yet read the full frame. */
			return 0;
		}
		if(ws_frame_is_masked(p_frame)) {
			a_mask[0] = p_frame[10];
			a_mask[1] = p_frame[11];
			a_mask[2] = p_frame[12];
			a_mask[3] = p_frame[13];
			inp_self->p_payload = &p_frame[14];
		}
		else {
			inp_self->p_payload = &p_frame[10];
		}
	}
   
	if(ws_frame_is_masked(p_frame)) {
		for(int i = 0; i < inp_self->payload_len; i++) {
			inp_self->p_payload[i] ^= a_mask[(i & 0x3)];
		}
	}
   
	return 1;
}

