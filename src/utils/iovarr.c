

#include <stdlib.h>
#include <string.h>

#include "iovarr.h"

struct _iovarr
{
	int refcount;
	int iovlen;
	struct iovec *iovarr;
};

iovarr_pt
iovarr_ctor(void)
{
	iovarr_pt p_self = calloc(1, sizeof(iovarr_t));
	if(p_self) {
		p_self->refcount = __sync_fetch_and_add(&p_self->refcount, 1);
	}
	return p_self;
}

static void
iovarr_free(void *inp)
{
	if(inp) {
		iovarr_pt p_self = (iovarr_pt)inp;
		p_self->refcount = __sync_fetch_and_sub(&p_self->refcount, 1);
		if(p_self->refcount == 0) {
			if(p_self->iovarr) {
				for(int i = 0; i < p_self->iovlen; i++) {
					if(p_self->iovarr[i].iov_base) {
						free(p_self->iovarr[i].iov_base);
					}
				}
				free(p_self->iovarr);
			}
			free(p_self);
		}
	}
}

void*
iovarr_popfront(iovarr_pt inp_self, int *outp_iov_len)
{
	if(inp_self && inp_self->iovlen > 0) {
		void *p = inp_self->iovarr[0].iov_base;
		if(outp_iov_len) *outp_iov_len = inp_self->iovarr[0].iov_len;
		if(inp_self->iovlen == 1) {
			inp_self->iovlen = 0;
			return p;
		}
		else {
			int moved =  inp_self->iovlen - 1;
			for(int i = 0; i < moved; i++) {
				void *dst = &inp_self->iovarr[i];
				void *src = &inp_self->iovarr[i+1];
				memcpy(dst, src, sizeof(struct iovec));
			}
			inp_self->iovlen--;
		}
		return p;
	}
	return NULL;
}

iovarr_pt
iovarr_copy_byref(iovarr_pt inp_self)
{
	if(inp_self) {
		inp_self->refcount = __sync_fetch_and_add(&inp_self->refcount, 1);
		return inp_self;
	}
	return NULL;
}

iovarr_pt
iovarr_incref(iovarr_pt inp_self)
{
	return iovarr_copy_byref(inp_self);
}

void
iovarr_decref(iovarr_pt inp_self)
{
	if(inp_self) {
		iovarr_free(inp_self);
	}
}

int
iovarr_pushback(iovarr_pt inp_self, struct iovec *inp_iovec)
{
	if(inp_self && inp_iovec) {
		int newlen = 0;
		if(inp_self->iovlen == 0) {
			if((inp_self->iovarr = calloc(1, sizeof(struct iovec))) == NULL) return -1;
			newlen = 1;
		}
		else {
			newlen = inp_self->iovlen + 1;
			inp_self->iovarr = realloc(inp_self->iovarr, newlen * sizeof(struct iovec));
			if(!inp_self->iovarr) return -1;
		}
		memcpy(&inp_self->iovarr[inp_self->iovlen], inp_iovec, sizeof(struct iovec));
		inp_self->iovlen = newlen;
		return 0;
	}
	return -1;
}

int
iovarr_steal(iovarr_pt inp_self, iovarr_pt inp_other)
{
	int newlen = 0;
	if(!inp_self || !inp_other) return -1;
	newlen = inp_self->iovlen + inp_other->iovlen;
	if(newlen > inp_self->iovlen) {
		inp_self->iovarr = inp_self->iovlen == 0 ?
			calloc(newlen, sizeof(struct iovec)) :
			realloc(inp_self->iovarr, newlen * sizeof(struct iovec));
		if(!inp_self->iovarr) return -1;	
		for (int i = 0; inp_other->iovlen > 0; i++, inp_other->iovlen--) {
			memcpy(&inp_self->iovarr[i+inp_self->iovlen], 
				&inp_other->iovarr[i], sizeof(struct iovec));
		}
		free(inp_other->iovarr);
		inp_other->iovarr = NULL;
		inp_self->iovlen = newlen;
	}
	return newlen;
}

int
iovarr_count(iovarr_pt inp_self)
{
	if(inp_self) return inp_self->iovlen;
	return -1;
}

struct iovec *
iovarr_ref(iovarr_pt inp_self)
{
	if(inp_self) return inp_self->iovarr;
	return NULL;
}


