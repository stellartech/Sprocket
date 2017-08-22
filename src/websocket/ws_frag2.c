
#include <event2/buffer.h>

#define WS_FRAG_FRIEND
#include "ws_frag2.h"

ws_frag_pt
ws_frag_ctor(void)
{
	ws_frag_pt p_self = calloc(1, sizeof(ws_frag_t));
	return p_self;
}

void
ws_frag_free(ws_frag_pt inp_self)
{
	if(inp_self) {
		if(inp_self->p_frag) {
			free(inp_self->p_frag);
		}
		if(inp_self->p_next) {
			ws_frag_free(inp_self->p_next);
		}
		free(inp_self);
	}
}

uint64_t
ws_frag_append_bufferevent(ws_frag_pt inp_self,
	struct bufferevent *inp_bev,
	ws_frag_append_bufferevent_rval_t *outp_rval)
{
	int is_first = 0;
	if(!inp_self) return 0;
	if(!inp_self->p_evbuffer) {
		is_first = 1;
		inp_self->p_evbuffer = evbuffer_new();
	}
	bufferevent_read_buffer(inp_bev, inp_self->p_evbuffer);
	if(is_first) {
		int offset = 0;
		char trash[16];
		evbuffer_copyout(inp_self->p_evbuffer, &inp_self->preable, sizeof(ws_frag_preamble_t));
		if((inp_self->preamble.short_len & 0x7f) < 126) {
			offset += 2;
		}
		else if((inp_self->preamble.short_len & 0x7f) == 126) {
			offset += 4;
		}
		else offset += 10;
		if((inp_self->preamble.short_len & 0x80) == 0x80) offset += 4;
		evbuffer_remove(inp_self->preamble.short_len, trash, offset + mask);
	}
	else {
		struct evbuffer *p_evbuf = bufferevent_get_input(inp_bev);
		evbuffer_add_buffer(inp_self->p_evbuffer, p_evbuf);
	}
	if(outp_rval) {
		outp_rval->new_len = evbuffer_len(inp_self->p_evbuffer);
	}

	int len;
	uint64_t new_len = 0;
	struct evbuffer *p_evbuf_in = bufferevent_get_input(inp_bev);
	while((len = evbuffer_get_length(p_evbuf_in)) > 0) {
		int len_read = 0;
		unsigned char *p = malloc(len);
		if(p) {
			while(len_read < len) {
				int i = 0;
				i = bufferevent_read(inp_bev, p, len - len_read);
				buffer_append(inp_self->payload, p, i);
				len_read += i;
				inp_self->frag_in += i;
			}
			free(p);
		}
		if(outp_rval) outp_rval->len_read += len_read;
	}
	if(outp_rval) {
		outp_rval->new_len = new_len;
	}
	return new_len;
}

int
ws_frag_is_unfrag(ws_frag_pt inp_self)
{
	if(inp_self) {
		return (inp_self->fin && inp_self->opcode > 0) ? 1 : 0;
	}
	return -1;
}

int
ws_frag_get_fin(ws_frag_pt inp_self)
{
	if(inp_self) {
		return inp_self->fin ? 1 : 0;
	}
	return -1;
}

int
ws_frag_get_opcode(ws_frag_pt inp_self)
{
	if(inp_self) {
		return inp_self->opcode;
	}
	return -1;
}

uint64_t
ws_frag_is_valid(ws_frag_pt inp_self)
{
	uint64_t offset = 0, mask = 0, rval = 0;
	if(!inp_self) return rval;
	if(inp_self->validated) return inp_self->payload.len;
	if(inp_self->frag_in > 1) {
		inp_self->fin       = inp_self->p_frag[0] & 0x80U == 0x80U ? 1 : 0;
		inp_self->opcode    = inp_self->p_frag[0] & 0x0fU;			
		inp_self->mask      = inp_self->p_frag[1] & 0x80U ? 1 : 0;
		inp_self->short_len = inp_self->p_frag[1] & 0x7fU ;
		if(inp_self->short_len < 126) {
			offset = 2;
			inp_self->payload.len = inp_self->short_len;
		}
		else if(inp_self->short_len == 126) {
			if(inp_self->frag_in > 3) {
				offset = 4;
				inp_self->payload.len = 0;
				inp_self->payload.bytes[0] = inp_self->p_frag[3];
				inp_self->payload.bytes[1] = inp_self->p_frag[2];
			}
			else {
				return 0; // Not enough data in buffer to complete
			}
		}
		else {
			if(inp_self->frag_in > 9) {
				offset = 10;
				inp_self->payload.len = 0;
				inp_self->payload.bytes[0] = inp_self->p_frag[9];
				inp_self->payload.bytes[1] = inp_self->p_frag[8];
				inp_self->payload.bytes[2] = inp_self->p_frag[7];
				inp_self->payload.bytes[3] = inp_self->p_frag[6];
				inp_self->payload.bytes[4] = inp_self->p_frag[5];
				inp_self->payload.bytes[5] = inp_self->p_frag[4];
				inp_self->payload.bytes[6] = inp_self->p_frag[3];
				inp_self->payload.bytes[7] = inp_self->p_frag[2];
			}
			else {
				return 0; // Not enough data in buffer to complete
			}
		}
		if(inp_self->mask) {
			mask = 4;
		}
		if(inp_self->frag_in >= (inp_self->payload.len + offset + mask)) {
			inp_self->p_payload = inp_self->p_frag + offset + mask;
			if(inp_self->mask) {
				unsigned char *p = (unsigned char *)inp_self->p_payload;
				unsigned char a_mask[4] = {0,0,0,0};
				a_mask[0] = inp_self->p_frag[offset+0];
				a_mask[1] = inp_self->p_frag[offset+1];
				a_mask[2] = inp_self->p_frag[offset+2];
				a_mask[3] = inp_self->p_frag[offset+3];
				for(uint64_t i = 0; i < inp_self->payload.len; i++) {
					p[i] = p[i] ^= a_mask[(i & 0x3)];
				}
			}
			inp_self->validated = 1;
			rval = inp_self->payload.len;
			// If we got more data in the input buffer than was for this
			// single websocket fragment/frame, start a new one with the
			// overflowed data as a chained fragment.
			if(inp_self->frag_in > (inp_self->payload.len + offset + mask) && !inp_self->fin) {
				inp_self->p_next = ws_frag_ctor();
				if(inp_self->p_next) {
					ws_frag_append_chunk(inp_self->p_next,
						inp_self->p_frag + inp_self->payload.len + offset + mask,
						inp_self->frag_in - inp_self->payload.len + offset + mask);
				}
				inp_self->p_frag = realloc(inp_self->p_frag, inp_self->payload.len + offset + mask);
			}
		}
	}
	return rval;
}

