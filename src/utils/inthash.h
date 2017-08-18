#ifndef INTHASH_H_INCLUDED
#define INTHASH_H_INCLUDED
  
#ifdef __cplusplus
extern "C" {
#endif
  
typedef void (*inthash_void_free_fn)(void*);
  
struct _inthash;
typedef struct _inthash    inthash_t;
typedef struct _inthash *  inthash_pt;
  
inthash_pt
inthash_ctor(int in_num_bins,
        inthash_void_free_fn in_free_fn);
  
static inline inthash_pt
inthash_new(void) {
	return inthash_ctor(32, NULL);
}

static inline inthash_pt
inthash_new_dtor(inthash_void_free_fn in_free_fn) {
	return inthash_ctor(32, in_free_fn);
}

size_t
inthash_count(inthash_pt inp_self);

void
inthash_decref(inthash_pt inp_self);
  
void
inthash_dtor(inthash_pt *inpp);
  
void*
inthash_find(inthash_pt inp_self, int in_index);

int
inthash_insert(inthash_pt inp_self, int in_index, void *inp_val);
  
void*
inthash_remove(inthash_pt inp_self, int in_index);
  
int
inthash_delete(inthash_pt inp_self, int in_index);

#ifdef __cplusplus
}
#endif

#endif /* INTHASH_H_INCLUDED */

