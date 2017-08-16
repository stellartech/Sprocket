#ifndef STRHASH_H_INCLUDED
#define STRHASH_H_INCLUDED
  
#include "str.h"

#ifdef __cplusplus
extern "C" {
#endif
  
typedef void (*strhash_void_free_fn)(void*);
  
struct _strhash;
typedef struct _strhash    strhash_t;
typedef struct _strhash *  strhash_pt;
  
strhash_pt
strhash_ctor(int in_num_bins,
        strhash_void_free_fn in_free_fn);
  
static inline strhash_pt
strhash_new(void) {
	return strhash_ctor(32, NULL);
}

static inline strhash_pt
strhash_new_dtor(strhash_void_free_fn in_free_fn) {
	return strhash_ctor(32, in_free_fn);
}

size_t
strhash_count(strhash_pt inp_self);

void
strhash_free(strhash_pt inp_self);
  
void
strhash_dtor(strhash_pt *inpp);
  
void*
strhash_findl(strhash_pt inp_self, str_pt inp_strkey);

void*
strhash_find(strhash_pt inp_self, str_pt inp_strkey);
  
int
strhash_insert(strhash_pt inp_self, str_pt inp_strkey, void *inp_val);
  
void*
strhash_remove(strhash_pt inp_self, str_pt inp_strkey);
  
int
strhash_delete(strhash_pt inp_self, str_pt inp_strkey);

#ifdef __cplusplus
}
#endif

#endif /* STRHASH_H_INCLUDED */

