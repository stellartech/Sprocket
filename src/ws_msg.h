

#ifndef WS_MSG_H_INCLUDED
#define WS_MSG_H_INCLUDED

#include <stdint.h>

#include <event2/bufferevent.h>

#define WS_FRAG_FRIEND
#include "ws_frag.h"

#ifdef WS_MSG_FRIEND
struct _ws_msg
{
	int state;
        int refcount;
	int frag_count;
	ws_frag_pt p_first_frag;
};
#else
struct _ws_msg;
#endif

typedef struct _ws_msg   ws_msg_t;
typedef struct _ws_msg * ws_msg_pt;

ws_msg_pt
ws_msg_ctor(void);

void
ws_msg_free(ws_msg_pt inp_self);

void
ws_msg_dtor(ws_msg_pt *inpp_self);

uint64_t
ws_msg_append_chunk(ws_msg_pt inp_self, 
	unsigned char *inp, uint64_t in_len);

uint64_t
ws_msg_append_bufferevent(ws_msg_pt inp_self, 
	struct bufferevent *inp_bev);

int
ws_msg_is_valid(ws_msg_pt inp_self);

uint64_t
ws_msg_memory_usage(ws_msg_pt inp_self);

#endif /* WS_MSG_H_INCLUDED */

