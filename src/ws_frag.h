

#ifndef WS_FRAG_H_INCLUDED
#define WS_FRAG_H_INCLUDED

#include <stdint.h>

#include <event2/bufferevent.h>

#ifdef WS_FRAG_FRIEND
struct _ws_frag
{
        int refcount;
	unsigned validated : 1;
	unsigned mask : 1;
	unsigned fin : 1;
	unsigned opcode : 4; 
	unsigned short_len : 7;
        unsigned char *p_frag;
        uint64_t frag_in;
	union {
		unsigned char bytes[8];
		uint64_t len;
	} payload __attribute__((packed));
        const unsigned char *p_payload;
	struct _ws_frag *p_next;
}; 
#else
struct _ws_frag;
#endif

#define DUMP_FRAG(x) \
	do {\
		ck_assert_msg(1==0, "mask:%u,fin:%u,op:%u,short_len:%u,len:%lx",x->mask,x->fin,x->opcode,x->short_len,x->payload.len);\
	} while(0)

typedef struct _ws_frag   ws_frag_t;
typedef struct _ws_frag * ws_frag_pt;

ws_frag_pt
ws_frag_ctor(void);

void
ws_frag_free(ws_frag_pt inp_self);

uint64_t
ws_frag_append_chunk(ws_frag_pt inp_self,
	unsigned char *inp, uint64_t in_len);

typedef struct
{
	uint64_t len_read;
	uint64_t new_len;
} ws_frag_append_bufferevent_rval_t;

uint64_t
ws_frag_append_bufferevent(ws_frag_pt inp_self,
	struct bufferevent *inp_bev,
	ws_frag_append_bufferevent_rval_t *outp_rval);

int
ws_frag_is_unfrag(ws_frag_pt inp_self);

int
ws_frag_get_opcode(ws_frag_pt inp_self);

uint64_t
ws_frag_is_valid(ws_frag_pt inp_self);


#endif /* WS_FRAG_H_INCLUDED */

