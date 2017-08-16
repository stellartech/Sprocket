#ifndef STR_H_INCLUDED
#define STR_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern C {
#endif

struct _str;
typedef struct _str   str_t;
typedef struct _str * str_pt;

str_pt
str_ctor(const char *inp, int in_len);

str_pt
str_copy_ctor(str_pt inp_self);

str_pt
str_steal_ctor(const char *inp, int in_len);

void
str_decref(str_pt in_str);

str_pt
str_copy_byref(str_pt inp_self);

const char*
str_get(str_pt inp_self);

const char*
str_get_with_len(str_pt inp_self, int *outp_len);

str_pt
str_concat(str_pt inp_self, str_pt inp_other);

int
str_get_len(str_pt inp_self);

int
str_get_refcount(str_pt inp_self);

int
str_get_hash(str_pt inp_self);

#ifdef __cplusplus
}
#endif

#endif /* STR_H_INCLUDED */
