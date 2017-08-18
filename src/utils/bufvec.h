
#ifndef BUFVEC_H_INCLUDED
#define BUFVEC_H_INCLUDED

#ifdef __cplusplus
extern C {
#endif

struct _bufvec;
typedef struct _bufvec   bufvec_t; 
typedef struct _bufvec * bufvec_pt; 

bufvec_pt
bufvec_ctor(int in_bufsize);

void
bufvec_free(void *inp_self);

void*
bufvec_addbuf(bufvec_pt inp_self, int in_bufsize);

bufvec_pt
bufvec_copy_byref(bufvec_pt inp_self);

#ifdef __cplusplus
}
#endif

#endif /* BUFVEC_H_INCLUDED */
