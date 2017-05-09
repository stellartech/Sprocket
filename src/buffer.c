
#include <stdlib.h>
#include <string.h>

#include "buffer.h"

#ifdef __cplusplus
extern C {
#endif


struct _buffer 
{
	int    refcount;
	int    len;
	void  *p_buf;	
};

buffer_pt
buffer_ctor(void)
{
	buffer_pt p_self = calloc(1, sizeof(buffer_t));
	return p_self;
}

buffer_pt
buffer_new(void* inp, int in_len)
{
	buffer_pt p_self = buffer_ctor();
	if(p_self) {
		p_self->p_buf = calloc(1, in_len + 1);
		p_self->refcount = 1;
		p_self->len = in_len;
		if(inp && in_len > 0) {
			memcpy(p_self->p_buf, inp, in_len);
		}
	}
	return p_self;
}

buffer_pt
buffer_new_byval(void* inp, int in_len)
{
	buffer_pt p_self = buffer_ctor();
	if(p_self) {
		p_self->p_buf = inp;
		p_self->len = in_len;
		p_self->refcount = 1;
	}
	return p_self;
}

void
buffer_free(buffer_pt inp_self)
{
	if(inp_self) {
		inp_self->refcount--;
		if(inp_self->refcount == 0) {
			if(inp_self->p_buf) free(inp_self->p_buf);
			free(inp_self);
		}
	}
}

void
buffer_dtor(buffer_pt *inpp_self)
{
	if(inpp_self) {
		buffer_pt p_self = *inpp_self;
		if(p_self) {
			int refcount = p_self->refcount - 1;
			buffer_free(p_self);
			if(refcount == 0) {
				*inpp_self = NULL;
			}
		}
	}
}

buffer_pt
buffer_copy_byref(buffer_pt inp_self)
{
	if(inp_self) {
		inp_self->refcount++;
		return inp_self;
	}
	return NULL;
}

const void*
buffer_ptr(buffer_pt inp_self)
{
	return inp_self ? (const void*)inp_self->p_buf : NULL;
}

int
buffer_len(buffer_pt inp_self)
{
	return inp_self ? inp_self->len : -1;
}

buffer_pt
buffer_append(buffer_pt inp_self, void *inp, int in_len)
{
	if(inp_self && in_len > 0) {
		int new_len = inp_self->len + in_len;
		if(new_len != inp_self->len) {
			inp_self->p_buf = inp_self->p_buf ?
				realloc(inp_self->p_buf, new_len + 1) :
				calloc(1, new_len + 1);	
		}
		memset((inp_self->p_buf + inp_self->len), 0, in_len + 1);
		memcpy((inp_self->p_buf + inp_self->len), inp, in_len);
		inp_self->len = new_len;
	}
	return inp_self;
}

int
buffer_is_equal(buffer_pt inp_self, buffer_pt inp_other)
{
	if(inp_self && inp_other) {
		if(inp_self->p_buf == inp_other->p_buf) return 0;
		if(inp_self->len == inp_other->len) {
			if(0 == memcmp(inp_self->p_buf, inp_other->p_buf, inp_self->len)) 
				return 0;
		}
	}
	return -1;	
}

buffer_pt
buffer_clone(buffer_pt inp_self)
{
	if(inp_self && inp_self->p_buf && inp_self->len > 0) {
		buffer_pt p_clone = buffer_new(inp_self->p_buf, inp_self->len);
	}
	return NULL;
}


#ifdef __cplusplus
}
#endif

