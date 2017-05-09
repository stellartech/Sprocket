
#include <stdlib.h>
#include <string.h>

#include <event2/buffer.h>

#define WS_MSG_FRIEND
#include "ws_msg.h"

ws_msg_pt
ws_msg_ctor(void)
{
	ws_msg_pt p_self = calloc(1, sizeof(ws_msg_t));
	if(p_self) {
		p_self->refcount = 1;
		p_self->p_first_frag = ws_frag_ctor();
		p_self->frag_count = 1;
	}	
	return p_self;
}

void
ws_msg_free(ws_msg_pt inp_self)
{
	if(inp_self) {
		inp_self->refcount--;
		if(inp_self->refcount < 1) {
			if(inp_self->p_first_frag) {
				ws_frag_free(inp_self->p_first_frag);
			}
			free(inp_self);
		}
	}
}

void
ws_msg_dtor(ws_msg_pt *inpp_self) 
{
	if(inpp_self) {
		ws_msg_pt p_self = *inpp_self;
		if(p_self) {
			int refcount = p_self->refcount - 1;
			ws_msg_free(p_self);
			if(refcount < 1) *inpp_self = NULL;
		}
	}
}

uint64_t
ws_msg_append_chunk(ws_msg_pt inp_self, 
	unsigned char *inp, uint64_t in_len)
{
	uint64_t len = 0;
	if(inp_self && inp_self->p_first_frag) {
		if(!ws_frag_is_valid(inp_self->p_first_frag)) {
			len = ws_frag_append_chunk(inp_self->p_first_frag, inp, in_len);
			ws_frag_is_valid(inp_self->p_first_frag);
		}
		else {
			ws_frag_pt p_last = ws_frag_chain_last(inp_self->p_first_frag);
			if(!ws_frag_is_valid(p_last)) {
				len = ws_frag_append_chunk(p_last, inp, in_len);
				ws_frag_is_valid(p_last);
			}
			else {
				ws_frag_pt p_new = ws_frag_ctor();
				p_new->p_next = NULL;
				p_last->p_next = p_new;
				len = ws_frag_append_chunk(p_new, inp, in_len);
				ws_frag_is_valid(p_new);
			}
		}
	}
	return len;
}

uint64_t
ws_msg_append_bufferevent(ws_msg_pt inp_self, 
	struct bufferevent *inp_bev)
{
	uint64_t len = 0;
	if(inp_self && inp_self->p_first_frag) {
		ws_frag_append_bufferevent_rval_t retval;
		if(!ws_frag_is_valid(inp_self->p_first_frag)) {
			ws_frag_append_bufferevent(inp_self->p_first_frag, inp_bev, &retval);
			ws_frag_is_valid(inp_self->p_first_frag);
		}
		else {
			ws_frag_pt p_last = ws_frag_chain_last(inp_self->p_first_frag);
			if(!ws_frag_is_valid(p_last)) {
				ws_frag_append_bufferevent(p_last, inp_bev, &retval);
				ws_frag_is_valid(p_last);
			}
			else {
				ws_frag_pt p_new = ws_frag_ctor();
				p_last->p_next = p_new;
				ws_frag_append_bufferevent(p_new, inp_bev, &retval);
				ws_frag_is_valid(p_new);
			}
		}
		len = retval.len_read;
	}
	return len;
}

int
ws_msg_is_valid(ws_msg_pt inp_self)
{
	if(inp_self && inp_self->p_first_frag) {
		ws_frag_pt p_frag = inp_self->p_first_frag;
		switch(ws_frag_chain_count(p_frag)) {
		case 0:
			return 0;
		case 1:
			if(ws_frag_is_valid(p_frag)) {
				if(ws_frag_is_unfrag(p_frag)) return 1;
				else return 0;
			}
			else {
				return 0;
			}
		default:
			while(p_frag) {
				if(!ws_frag_is_valid(p_frag)) return 0;
				if(!ws_frag_is_unfrag(p_frag)) {
					p_frag = p_frag->p_next;
					continue;
				}
				return 1;
			}
		}
	}
	return -1;
}

uint64_t 
ws_msg_memory_usage(ws_msg_pt inp_self)
{
	if(inp_self && inp_self->p_first_frag) {
		return ws_frag_chain_fraglen(inp_self->p_first_frag);
	}
	return 0;
}

