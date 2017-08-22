

#ifndef WS_WIRE_H_INCLUDED
#define WS_WIRE_H_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <nanomsg/nn.h>
#include <event2/buffer.h>

#ifdef __cplusplus
extern C {
#endif

typedef struct
{
	unsigned fin:1;
	unsigned opcode:1;
	uint64_t payload_len;
} ws_vec_info_t, *ws_vec_info_pt;

typedef struct
{
	uint64_t total_size;
	unsigned fin : 1;
	unsigned opcode : 4;
	ws_vec_info_pt p_vecinfo;
	struct nn_msghdr msghdr;
} ws_nn_msghdr_t, *ws_nn_msghdr_pt;

static inline ws_nn_msghdr_pt
ws_nn_msghdr_new(int veccnt)
{
	ws_nn_msghdr_pt p_self = calloc(1, sizeof(ws_nn_msghdr_t));
	if(p_self) {
		if(veccnt > 0) {
			p_self->msghdr.msg_iov = calloc(veccnt, sizeof(struct nn_iovec));
			p_self->msghdr.msg_iovlen = veccnt;
			p_self->p_vecinfo = calloc(veccnt, sizeof(ws_vec_info_t));
		}
		p_self->msghdr.msg_control = NULL;
		p_self->msghdr.msg_controllen = 0;
	}
	return p_self;
}

static void
ws_nn_msghdr_free(ws_nn_msghdr_pt inp) 
{
	if(inp) {
		if(inp->msghdr.msg_iov) free(inp->msghdr.msg_iov);
		if(inp->p_vecinfo) free(inp->p_vecinfo);
		if(inp->msghdr.msg_control) free(inp->msghdr.msg_control);
		free(inp);
	}
}

static void
ws_nn_msghdr_free_full(ws_nn_msghdr_pt inp) 
{
	if(inp) {
		for(int i = 0; i < inp->msghdr.msg_iovlen; i++) {
			if(inp->msghdr.msg_iov[i].iov_base) {
				nn_freemsg(inp->msghdr.msg_iov[i].iov_base);
			}
		}
		ws_nn_msghdr_free(inp);
	}
}

static int 
ws_nn_msghdr_num_of_vecs(ws_nn_msghdr_pt inp_self)
{
	if(inp_self) {
		return inp_self->msghdr.msg_iovlen;
	}
	return -1;
};
static void
ws_nn_msghdr_new_vec_size(ws_nn_msghdr_pt inp_self, int newsize)
{
	if(inp_self && inp_self->msghdr.msg_iov) {
		int new_arr_size = inp_self->msghdr.msg_iovlen + newsize;
		if(!inp_self->msghdr.msg_iov) {
			inp_self->msghdr.msg_iov = calloc(newsize, sizeof(struct nn_iovec));
			inp_self->msghdr.msg_iovlen = newsize;
		}
		else {
			void *p_new_section_start = NULL;
			inp_self->msghdr.msg_iov = realloc(inp_self->msghdr.msg_iov, 
				sizeof(struct nn_iovec) * new_arr_size);	
			p_new_section_start = inp_self->msghdr.msg_iov;
			p_new_section_start += (inp_self->msghdr.msg_iovlen * sizeof(struct nn_iovec));
			memset(p_new_section_start, 0, newsize * sizeof(struct nn_iovec));

		}
		if(!inp_self->p_vecinfo) {
			inp_self->p_vecinfo = calloc(newsize, sizeof(ws_vec_info_t));
		}
		else {
			void *p_new_section_start = NULL;
			inp_self->p_vecinfo = realloc(inp_self->p_vecinfo,
				sizeof(ws_vec_info_t) * new_arr_size);
			p_new_section_start = inp_self->p_vecinfo;
			p_new_section_start += (inp_self->msghdr.msg_iovlen * sizeof(ws_vec_info_t));
			memset(p_new_section_start, 0, newsize * sizeof(ws_vec_info_t));
		}
		inp_self->msghdr.msg_iovlen = new_arr_size;
	}
}

static int  
ws_nn_msghdr_vec_inc(ws_nn_msghdr_pt inp_self)
{
	int newsize = 0, rval = 0;
	if(inp_self) {
		rval = inp_self->msghdr.msg_iovlen;
		newsize = inp_self->msghdr.msg_iovlen + 1;
		ws_nn_msghdr_new_vec_size(inp_self, newsize);
	}
	return rval;
}

ws_nn_msghdr_pt
ws_wire_process(struct evbuffer *inp_evbuffer, ws_nn_msghdr_pt inp_msghdr, int nn_flags);

#ifdef __cplusplus
}
#endif



#endif /* WS_WIRE_H_INCLUDED */

