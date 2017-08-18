#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED
  
#ifdef __cplusplus
extern "C" {
#endif
  
typedef void (*hashmap_void_free_fn)(void*);
  
struct _hashmap;
typedef struct _hashmap    hashmap_t;
typedef struct _hashmap *  hashmap_pt;
  
hashmap_pt
hashmap_ctor(int in_num_bins,
        hashmap_void_free_fn in_free_fn);
  
static inline hashmap_pt
hashmap_new(void) {
	return hashmap_ctor(32, NULL);
}

static inline hashmap_pt
hashmap_new_dtor(hashmap_void_free_fn in_free_fn) {
	return hashmap_ctor(32, in_free_fn);
}

size_t
hashmap_count(hashmap_pt inp_self);

void
hashmap_free(hashmap_pt inp_self);
  
void
hashmap_dtor(hashmap_pt *inpp);
  
void*
hashmap_findl(hashmap_pt inp_self, 
	const char *inp_key, int in_key_len);

void*
hashmap_find(hashmap_pt inp_self,
        const char *inp_key);
  
int
hashmap_insert(hashmap_pt inp_self,
        const char *inp_key, void *inp_val);
  
void*
hashmap_remove(hashmap_pt inp_self,
        const char *inp_key);
  
int
hashmap_delete(hashmap_pt inp_self, 
	const char *inp_key);

#ifdef __cplusplus
}
#endif

#endif /* HASHMAP_H_INCLUDED */

