
#ifndef BUFFER_H_INCLUDED
#define BUFFER_H_INCLUDED

#ifdef __cplusplus
extern C {
#endif


struct _buffer;
typedef struct _buffer   buffer_t; 
typedef struct _buffer * buffer_pt; 

buffer_pt
buffer_ctor(void);

buffer_pt
buffer_new(void* inp, int in_len);

void
buffer_free(buffer_pt inp_self);

void
buffer_dtor(buffer_pt *inpp_self);

const void*
buffer_ptr(buffer_pt inp_self);

int
buffer_len(buffer_pt inp_self);

buffer_pt
buffer_append(buffer_pt inp_self, void *inp, int in_len);

buffer_pt
buffer_clear(buffer_pt inp_self);


#ifdef __cplusplus
}
#endif

#endif /* BUFFER_H_INCLUDED */
