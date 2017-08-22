

#ifndef WS_FRAME_H_INCLUDED
#define WS_FRAME_H_INCLUDED

#include <stdint.h>

#include <event2/bufferevent.h>

#ifdef WS_FRAME_FRIEND
struct _ws_frame
{
        int refcount;
        unsigned char *p_frame;
        uint64_t frame_in;
        unsigned char *p_payload;
        uint64_t payload_len;
};
#else
struct _ws_frame;
#endif

typedef struct _ws_frame   ws_frame_t;
typedef struct _ws_frame * ws_frame_pt;

ws_frame_pt
ws_frame_ctor(void);

void
ws_frame_free(ws_frame_pt inp_self);

void
ws_frame_dtor(ws_frame_pt *inpp_self);

uint64_t
ws_frame_append_chunk(ws_frame_pt inp_self, 
	unsigned char *inp, uint64_t in_len);

uint64_t
ws_frame_append_bufferevent(ws_frame_pt inp_self, 
	struct bufferevent *inp_bev);

int
ws_frame_is_valid(ws_frame_pt inp_self);

#endif /* WS_FRAME_H_INCLUDED */

