


#ifndef IOVARR_H_INCLUDED
#define IOVARR_H_INCLUDED

#include <sys/uio.h>

struct _iovarr;
typedef struct _iovarr   iovarr_t;
typedef struct _iovarr * iovarr_pt;

iovarr_pt
iovarr_ctor(void);

void
iovarr_free(void*);

iovarr_pt
iovarr_incref(iovarr_pt inp_self);

void
iovarr_decref(iovarr_pt inp_self);

int
iovarr_count(iovarr_pt inp_self);

struct iovec *
iovarr_ref(iovarr_pt inp_self);

void*
iovarr_popfront(iovarr_pt inp_self, int *outp_len);

int
iovarr_pushback(iovarr_pt inp_self, struct iovec *inp_iovec);

#endif /* IOVARR_H_INCLUDED */

