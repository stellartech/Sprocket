
#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern C {
#endif


struct _buffer;
typedef struct _buffer   buffer_t; 
typedef struct _buffer * buffer_pt; 

buffer_pt
buffer_ctor(void);

buffer_pt
buffer_new(void* inp, uint64_t in_len);

buffer_pt
buffer_new_byval(void* inp, uint64_t in_len);

void
buffer_free(buffer_pt inp_self);

void
buffer_dtor(buffer_pt *inpp_self);

const void*
buffer_ptr(buffer_pt inp_self);

uint64_t
buffer_len(buffer_pt inp_self);

buffer_pt
buffer_append(buffer_pt inp_self, void *inp, uint64_t in_len);

uint64_t 
buffer_equals(buffer_pt inp_self, buffer_pt inp_other);

buffer_pt
buffer_clone(buffer_pt inp_self);

buffer_pt
buffer_copy_byref(buffer_pt inp_self);

#ifdef __cplusplus
}
#endif

#endif /* BUFFER_H_INCLUDED */
