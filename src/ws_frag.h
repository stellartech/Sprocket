

#ifndef WS_FRAG_H_INCLUDED
#define WS_FRAG_H_INCLUDED

#include <stdint.h>

#include <event2/bufferevent.h>

#ifdef WS_FRAG_FRIEND
struct _ws_frag
{
        int refcount;
        unsigned char *p_frag;
        uint64_t frag_in;
        unsigned char *p_payload;
        uint64_t payload_len;
	struct _ws_frag *p_next;
};
#else
struct _ws_frag;
#endif

typedef struct _ws_frag   ws_frag_t;
typedef struct _ws_frag * ws_frag_pt;

ws_frag_pt
ws_frag_ctor(void);

void
ws_frag_free(ws_frag_pt inp_self);

void
ws_frag_dtor(ws_frag_pt *inpp_self);

#endif /* WS_FRAG_H_INCLUDED */

