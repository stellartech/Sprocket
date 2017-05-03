
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
  
#include "hashmap.h"
  
#ifdef __cplusplus
extern "C" {
#endif

static inline uint32_t hash_func(const char *inp_key, uint32_t in_len);
  
struct _hashmap_bin;
typedef struct _hashmap_bin    hashmap_bin_t;
typedef struct _hashmap_bin *  hashmap_bin_pt;
typedef struct _hashmap_bin ** hashmap_bin_ppt;
  
struct _hashmap_bin
{
	uint64_t        hash;
	const char*     p_key;
	void*           p_value;
	hashmap_bin_pt  p_next;
};
  
struct _hashmap
{
	size_t                num_of_entries;
	int                   num_of_bins;
	int                   bin_mask;
	hashmap_void_free_fn  p_bin_void_free_fn;
	hashmap_bin_ppt       p_bins;
};
  
hashmap_pt
hashmap_ctor(int in_num_bins, hashmap_void_free_fn in_free_fn)
{
    hashmap_pt p_self = (hashmap_pt)calloc(1, sizeof(hashmap_t));
    if(p_self) {
        int i = 3;
        p_self->num_of_bins = in_num_bins;
        if(in_num_bins >= 0x80000000) {
            p_self->num_of_bins = 0x80000000;
        }
        else {
            while((1U << i) < in_num_bins) {
                ++i;
            }
            p_self->num_of_bins = 1 << i;
        }
        p_self->bin_mask = p_self->num_of_bins - 1;
        p_self->p_bin_void_free_fn = in_free_fn;
        p_self->p_bins = (hashmap_bin_ppt)calloc(p_self->num_of_bins,
                sizeof(hashmap_bin_ppt));
        if(!p_self->p_bins) {
            free(p_self);
            return NULL;
        }
    }
    return p_self;
}
  
static hashmap_bin_pt
hashmap_bin_free(hashmap_bin_pt inp_self, hashmap_void_free_fn in_fnf)
{
    hashmap_bin_pt p_next = NULL;
    if(inp_self) {
        p_next = inp_self->p_next;
        if(in_fnf && inp_self->p_value) {
            (in_fnf)(inp_self->p_value);
        }
        if(inp_self->p_key) free((void*)inp_self->p_key);
        free(inp_self);
    }
    return p_next;
}
  
void
hashmap_free(hashmap_pt inp_self)
{
    if(inp_self) {
        int i;
        for(i = 0; i < inp_self->num_of_bins; i++) {
            hashmap_bin_pt p_next = hashmap_bin_free(inp_self->p_bins[i],
                    inp_self->p_bin_void_free_fn);
            inp_self->p_bins[i] = 0;
            while(p_next != NULL) {
                p_next = hashmap_bin_free(p_next,
                        inp_self->p_bin_void_free_fn);
            }
        }
        free(inp_self->p_bins);
        free(inp_self);
    }
}
  
void
hashmap_dtor(hashmap_pt *inpp)
{
    if(inpp) {
        hashmap_pt p_self = *inpp;
        if(p_self) hashmap_free(p_self);
        *inpp = NULL;
    }
}
  
void*
hashmap_find(hashmap_pt inp_self, const char *inp_key)
{
    uint32_t hash = hash_func(inp_key, strlen(inp_key));
    int bin = hash & inp_self->bin_mask;
    hashmap_bin_pt p_bin = (hashmap_bin_pt)inp_self->p_bins[bin];
    while(p_bin) {
        if(p_bin->hash == hash && !strcmp(inp_key, p_bin->p_key)) {
            return p_bin->p_value;
        }
        p_bin = p_bin->p_next;
    }
    return NULL;
}
  
int
hashmap_insert(hashmap_pt inp_self, const char *inp_key, void *inp_val)
{
	uint32_t hash = hash_func(inp_key, strlen(inp_key));
	int bin = hash & inp_self->bin_mask;
	hashmap_bin_pt p_bin = calloc(1, sizeof(hashmap_bin_t));
	if(!p_bin) {
		return -1;
	}
	p_bin->hash = hash;
	p_bin->p_value = inp_val;
	p_bin->p_key = strdup(inp_key);
	if(!inp_self->p_bins[bin]) {
		inp_self->p_bins[bin] = p_bin;
	}
	else {
		p_bin->p_next = inp_self->p_bins[bin];
		inp_self->p_bins[bin] = p_bin;
	}
	inp_self->num_of_entries++;
	return 0;
}
  
void*
hashmap_remove(hashmap_pt inp_self, const char *inp_key)
{
	void *p_rval = NULL;
	uint32_t hash = hash_func(inp_key, strlen(inp_key));
	int bin = hash & inp_self->bin_mask;
	hashmap_bin_pt p_prev = NULL, p_bin = inp_self->p_bins[bin];
	while(p_bin) {
		if(p_bin->hash == hash && !strcmp(inp_key, p_bin->p_key)) {
			if(p_prev) {
				p_prev->p_next = p_bin->p_next;
			}
			else {
				inp_self->p_bins[bin] = p_bin->p_next;
			}
			inp_self->num_of_entries--;
			p_rval = p_bin->p_value;
			hashmap_bin_free(p_bin, NULL);
			return p_rval;
		}
		p_prev = p_bin;
		p_bin = p_bin->p_next;
	}
	return p_rval;;
}
  
void
hashmap_delete(hashmap_pt inp_self, const char *inp_key)
{
	uint32_t hash = hash_func(inp_key, strlen(inp_key));
	int bin = hash & inp_self->bin_mask;
	hashmap_bin_pt p_prev = NULL, p_bin = inp_self->p_bins[bin];
	while(p_bin) {
		if(p_bin->hash == hash && !strcmp(inp_key, p_bin->p_key)) {
			if(p_prev) {
				p_prev->p_next = p_bin->p_next;
			}
			else {
				inp_self->p_bins[bin] = p_bin->p_next;
			}
			hashmap_bin_free(p_bin, inp_self->p_bin_void_free_fn);
			inp_self->num_of_entries--;
			return;
		}
		p_prev = p_bin;
	p_bin = p_bin->p_next;
	}
	return;
}

size_t
hashmap_count(hashmap_pt inp_self)
{
	return inp_self->num_of_entries;
}
  
static inline uint32_t
hash_func(const char *inp_key, uint32_t in_len)
{
	register uint32_t hash = 5381;
	/* variant with the hash unrolled eight times */
	for (; in_len >= 8; in_len -= 8) {
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
		hash = ((hash << 5) + hash) + *inp_key++;
	}
	switch (in_len) {
		case 7: hash = ((hash << 5) + hash) + *inp_key++; /* fallthrough... */
		case 6: hash = ((hash << 5) + hash) + *inp_key++; /* fallthrough... */
		case 5: hash = ((hash << 5) + hash) + *inp_key++; /* fallthrough... */
		case 4: hash = ((hash << 5) + hash) + *inp_key++; /* fallthrough... */
		case 3: hash = ((hash << 5) + hash) + *inp_key++; /* fallthrough... */
		case 2: hash = ((hash << 5) + hash) + *inp_key++; /* fallthrough... */
		case 1: hash = ((hash << 5) + hash) + *inp_key++; break;
		case 0: default: break;
	}
	return hash;
}

#ifdef __cplusplus
}
#endif

