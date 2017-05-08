
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
	}	
	return p_self;
}

void
ws_msg_free(ws_msg_pt inp_self)
{
	if(inp_self) {

		free(inp_self);
	}
}

uint64_t
ws_msg_append_chunk(ws_msg_pt inp_self, 
	unsigned char *inp, uint64_t in_len)
{
	uint64_t len = 0;

	return len;
}

uint64_t
ws_msg_append_bufferevent(ws_msg_pt inp_self, 
	struct bufferevent *inp_bev)
{
	uint64_t len = 0;

	return len;
}

int
ws_msg_is_valid(ws_msg_pt inp_self)
{
	return 1;
}


