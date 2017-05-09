

#include <stdlib.h>

#include "ws_frame_buffer.h"

#ifdef __cplusplus
extern C {
#endif


// Constructor
ws_frame_buffer_pt
ws_frame_buffer_ctor(void)
{
	ws_frame_buffer_pt p_self = calloc(1, sizeof(ws_frame_buffer_t));
	if(p_self) {
		if((p_self->p_evbuffer = evbuffer_new()) == NULL) {
			free(p_self);
			return NULL;
		}
	}
	return p_self;
}

// Free function
void
ws_frame_buffer_free(void *inp)
{
	ws_frame_buffer_pt p_self = (ws_frame_buffer_pt)inp;
	if(p_self) {
		if(p_self->p_evbuffer) evbuffer_free(p_self->p_evbuffer);
		if(p_self->p_next) ws_frame_buffer_free(p_self->p_next);
		free(p_self);
	}	
}

// Destructor
void
ws_frame_buffer_dtor(ws_frame_buffer_pt *inpp_self)
{
	if(inpp_self) {
		ws_frame_buffer_pt p_self = (ws_frame_buffer_pt)*inpp_self;
		if(p_self) {
			ws_frame_buffer_free(p_self);
		}
		*inpp_self = NULL;
	}
}

static int
ws_frame_payload_len(ws_frame_buffer_pt inp_self)
{
	if(inp_self) {
		if(inp_self->mask == 0) {
			switch(inp_self->type) {
			case 1: return (int)(inp_self->preamble.short_len & 0x7f);
			case 2: return (int)inp_self->preamble.extended.len126.len;
			case 3: return (int)inp_self->preamble.extended.len127.len;
			default: return -1;
			}
		}
		else {
			switch(inp_self->type) {
			case 1: return (int)(inp_self->preamble.short_len & 0x7f);
			case 2: return (int)inp_self->preamble.extended.m_len126.len.len;
			case 3: return (int)inp_self->preamble.extended.m_len127.len.len;
			default: return -1;
			}
		}
	}
	return -2;
}

static int 
ws_frame_append_process_type(ws_frame_buffer_pt inp_self)
{
	unsigned char short_len = 0;
	size_t len = evbuffer_get_length(inp_self->p_evbuffer);
	ev_ssize_t copied = evbuffer_copyout(inp_self->p_evbuffer, &inp_self->preamble, sizeof(ws_frame_preamble_t));
	if(copied > 1) {
		short_len = inp_self->preamble.short_len & 0x7f;
		inp_self->mask =  ((inp_self->preamble.short_len & 0x80) == 0x80) ? 4 : 0;
		if(short_len < 126) {
			inp_self->offset = 2;
			inp_self->mask =  ((inp_self->preamble.short_len & 0x80) == 0x80) ? 4 : 0;
			inp_self->type = 1;
		}
	}
	if(copied > 3) {
		if(short_len == 126) {
			inp_self->offset = 4;
			inp_self->type = 2;
		}
	}
	if(copied > 9) {
		if(short_len == 127) {
			inp_self->offset = 10;
			inp_self->type = 3;
		}
	}
	return copied;
}

static void
ws_frame_append_process_length(ws_frame_buffer_pt inp_self)
{
	int buffer_len = evbuffer_get_length(inp_self->p_evbuffer);
	int payload_len = ws_frame_payload_len(inp_self);
	int total_len = payload_len + inp_self->offset + inp_self->mask;
	if(buffer_len >= total_len) {
		inp_self->complete = 1;
	} 		
	if(buffer_len > total_len) {
		// Unlikely but handle any overflow that crept into this buffer space.
		// ToDo. Not obvious how to trim some "excess" bytes from a evbuffer 
		// using it's API. More research here to close off this edge case.
		// But it does mean the next incoming message is now mangled beyond
		// repair as we have part of it's beginning!
	}
}

int
ws_frame_append(ws_frame_buffer_pt inp_self, struct evbuffer *inp_evbuffer)
{
	if(!inp_self) return -1;
	if(!inp_self->p_evbuffer) {
		if(!(inp_self->p_evbuffer = evbuffer_new())) return -2;
		if(!(evbuffer_add_buffer(inp_self->p_evbuffer, inp_evbuffer))) {
			evbuffer_free(inp_self->p_evbuffer);
			inp_self->p_evbuffer = NULL;
			return -3;
		}
	}
	else {
		if(!(evbuffer_add_buffer(inp_self->p_evbuffer, inp_evbuffer))) {
			evbuffer_free(inp_self->p_evbuffer);
			inp_self->p_evbuffer = NULL;
			return -4;
		}
	}
	if(inp_self->type == 0) {
		ws_frame_append_process_type(inp_self);
	}
	if(inp_self->type != 0 && !inp_self->complete) {
		ws_frame_append_process_length(inp_self);
	}
		
	return 0;
}


#ifdef __cplusplus
}
#endif




