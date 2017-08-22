

#ifndef WS_FRAG_H_INCLUDED
#define WS_FRAG_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <event2/bufferevent.h>

#ifndef WS_FRAG_FRIEND
struct _ws_frag;
typedef struct _ws_frag   ws_frag_t;
typedef struct _ws_frag * ws_frag_pt;
#else
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
typedef struct _ws_frag   ws_frag_t;
typedef struct _ws_frag * ws_frag_pt;

static inline int
ws_frag_chain_count(ws_frag_pt inp)
{
	int rval = 0;
	while(inp) {
		rval++;
		inp = inp->p_next;
	}
	return rval;
}

static inline ws_frag_pt
ws_frag_chain_last(ws_frag_pt inp) 
{
	while(inp && inp->p_next) inp = inp->p_next;
	return inp;
}

static inline uint64_t 
ws_frag_chain_len(ws_frag_pt inp)
{
	uint64_t rval = 0;
	while(inp) {
		rval += inp->payload.len;
		inp = inp->p_next;
	}
	return rval;
}

static inline uint64_t 
ws_frag_chain_fraglen(ws_frag_pt inp)
{
	uint64_t rval = 0;
	while(inp) {
		rval += inp->frag_in;
		inp = inp->p_next;
	}
	return rval;
}

static unsigned char*
ws_frag_chain_pullup(ws_frag_pt inp, uint64_t *outp_len)
{
	uint64_t len = 0, so_far = 0;
	unsigned char *p_rval = NULL;
	if((len = ws_frag_chain_len(inp)) > 0) {
		if((p_rval = calloc(1, len)) != NULL) {
			while(inp && so_far < len) {
				memcpy(p_rval + so_far, inp->p_payload, inp->payload.len);
				so_far += inp->payload.len;
				inp = inp->p_next;	
			}
		}
		if(outp_len != NULL) *outp_len = len;
	}
	return p_rval;
}
#endif

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

