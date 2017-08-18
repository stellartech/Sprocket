

#include <stdlib.h>
#include <sys/uio.h>

#include "bufvec.h"

#ifdef __cplusplus
extern C {
#endif

struct _bufvec
{
	int iovlen;
	struct iovec *p_vecs;
};

bufvec_pt
bufvec_ctor(int in_len)
{
	bufvec_pt p_self;
	if((p_self = calloc(1, sizeof(bufvec_t))) == NULL)
		goto bufvec_ctor_fail;
	if((p_self->p_vecs = calloc(1, sizeof(struct iovec))) == NULL) 
		goto bufvec_ctor_fail;
	p_self->iovlen = 1;
	if((p_self->p_vecs[0].iov_base = calloc(1, in_len)) == NULL)
		goto bufvec_ctor_fail;	
	p_self->p_vecs[0].iov_len = 0;
	return p_self;
	bufvec_ctor_fail:
	bufvec_free(p_self);
	return NULL;
}

void
bufvec_free(void *inp_self)
{
	if(inp_self) {
		bufvec_pt p_self = (bufvec_pt)inp_self;
		if(p_self->p_vecs) {
			for(int i = 0; i < p_self->iovlen; i++) {
				if(p_self->p_vecs[i].iov_base != NULL) 
					free(p_self->p_vecs[i].iov_base);
			}
			free(p_self->p_vecs);
		}
		free(inp_self);
	}
}

void*
bufvec_addbuf(bufvec_pt inp_self, int in_bufsize)
{
	if(inp_self) {
		void *prval;
		int new_iovlen = inp_self->iovlen + 1;
		if((prval = calloc(1, in_bufsize)) == NULL) return NULL;
		inp_self->p_vecs = realloc(inp_self->p_vecs, (new_iovlen * sizeof(struct iovec)));
		if(!inp_self->p_vecs) {
			free(prval);
			return NULL;
		}
		inp_self->p_vecs[inp_self->iovlen].iov_base = prval;
		inp_self->p_vecs[inp_self->iovlen].iov_len = in_bufsize;
		inp_self->iovlen = new_iovlen;
		return prval;
	}	
	return NULL;
}

bufvec_pt
bufvec_copy_byref(bufvec_pt inp_self)
{
	return inp_self;
}

#ifdef __cplusplus
}
#endif

