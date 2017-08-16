
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
  
#include "strhash.h"
  
#ifdef __cplusplus
extern "C" {
#endif
  
struct _strhash_bin;
typedef struct _strhash_bin    strhash_bin_t;
typedef struct _strhash_bin *  strhash_bin_pt;
typedef struct _strhash_bin ** strhash_bin_ppt;
  
struct _strhash_bin
{
	int	        hash;
	str_pt		p_key;
	void*           p_value;
	strhash_bin_pt  p_next;
};
  
struct _strhash
{
	size_t                num_of_entries;
	int                   num_of_bins;
	int                   bin_mask;
	strhash_void_free_fn  p_bin_void_free_fn;
	strhash_bin_ppt       p_bins;
	pthread_mutex_t       lock;
};
  
strhash_pt
strhash_ctor(int in_num_bins, strhash_void_free_fn in_free_fn)
{
	strhash_pt p_self = (strhash_pt)calloc(1, sizeof(strhash_t));
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
		p_self->p_bins = (strhash_bin_ppt)calloc(p_self->num_of_bins,
			sizeof(strhash_bin_ppt));
		if(!p_self->p_bins) {
			goto strhash_ctor_fail;
		}
		if(pthread_mutex_init(&p_self->lock, NULL) != 0) {
			goto strhash_ctor_fail;
		}
	}
	return p_self;

strhash_ctor_fail:
	if(p_self) {
		strhash_dtor(&p_self);
	}
	return NULL;
}
  
static strhash_bin_pt
strhash_bin_free(strhash_bin_pt inp_self, strhash_void_free_fn in_fnf)
{
	strhash_bin_pt p_next = NULL;
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
strhash_free(strhash_pt inp_self)
{
	if(inp_self) {
		pthread_mutex_lock(&inp_self->lock);	
		for(int i = 0; i < inp_self->num_of_bins; i++) {
			strhash_bin_pt p_next = strhash_bin_free(inp_self->p_bins[i],
			   inp_self->p_bin_void_free_fn);
			inp_self->p_bins[i] = 0;
			while(p_next != NULL) {
				p_next = strhash_bin_free(p_next,
				   inp_self->p_bin_void_free_fn);
			}
		}
		pthread_mutex_unlock(&inp_self->lock);
		pthread_mutex_destroy(&inp_self->lock);
		free(inp_self->p_bins);
		free(inp_self);
	}
}
  
void
strhash_dtor(strhash_pt *inpp)
{
	if(inpp) {
		strhash_pt p_self = *inpp;
		if(p_self) strhash_free(p_self);
		*inpp = NULL;
	}
}
  
void*
strhash_find(strhash_pt inp_self, str_pt inp_strkey) 
{
	int bin;
	void *p_rval = NULL;
	int hash = str_get_hash(inp_strkey);
	int search_key_len = str_get_len(inp_strkey);
	const char *p_search_key = str_get(inp_strkey);
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return NULL;
	}
	bin = hash & inp_self->bin_mask;
	strhash_bin_pt p_bin = (strhash_bin_pt)inp_self->p_bins[bin];
	while(p_bin) {
		const char *p_found_key = str_get(p_bin->p_key);
		if(p_bin->hash == hash && strncmp(p_search_key, p_found_key, search_key_len) == 0) {
			p_rval = p_bin->p_value;
			pthread_mutex_unlock(&inp_self->lock);
			return p_rval; 
		}
		p_bin = p_bin->p_next;
	}
	pthread_mutex_unlock(&inp_self->lock);
	return p_rval;
}
  
void*
strhash_find_ex(strhash_pt inp_self, const char *inp_key)
{
	void *p_rval = NULL;
	str_pt p_temp = str_ctor(inp_key, strlen(inp_key));
	p_rval = strhash_find(inp_self, p_temp);
	str_free(p_temp);
	return p_rval;
}
  
int
strhash_insert(strhash_pt inp_self, str_pt inp_strkey, void *inp_val) //const char *inp_key, void *inp_val)
{
	int bin;
	int hash = str_get_hash(inp_strkey); //hash_func(inp_key, strlen(inp_key));
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return -1;
	}
	bin = hash & inp_self->bin_mask;
	strhash_bin_pt p_bin = calloc(1, sizeof(strhash_bin_t));
	if(!p_bin) {
		pthread_mutex_unlock(&inp_self->lock);
		return -1;
	}
	p_bin->hash = hash;
	p_bin->p_value = inp_val;
	p_bin->p_key = str_copy_byref(inp_strkey);
	if(!inp_self->p_bins[bin]) {
		inp_self->p_bins[bin] = p_bin;
	}
	else {
		p_bin->p_next = inp_self->p_bins[bin];
		inp_self->p_bins[bin] = p_bin;
	}
	inp_self->num_of_entries++;
	pthread_mutex_unlock(&inp_self->lock);
	return 0;
}
  
void*
strhash_remove(strhash_pt inp_self, str_pt inp_strkey) //const char *inp_key)
{
	int bin;
	void *p_rval = NULL;
	int hash = str_get_hash(inp_strkey); //hash_func(inp_key, strlen(inp_key));
	int search_key_len = str_get_len(inp_strkey);
	const char *p_search_key = str_get(inp_strkey);
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return NULL;
	}
	p_rval = NULL;
	bin = hash & inp_self->bin_mask;
	strhash_bin_pt p_prev = NULL, p_bin = inp_self->p_bins[bin];
	while(p_bin) {
		const char *p_found_key = str_get(p_bin->p_key);
		if(p_bin->hash == hash && strncmp(p_search_key, p_found_key, search_key_len) == 0) {
			if(p_prev) {
				p_prev->p_next = p_bin->p_next;
			}
			else {
				inp_self->p_bins[bin] = p_bin->p_next;
			}
			inp_self->num_of_entries--;
			p_rval = p_bin->p_value;
			strhash_bin_free(p_bin, NULL);
			pthread_mutex_unlock(&inp_self->lock);
			return p_rval;
		}
		p_prev = p_bin;
		p_bin = p_bin->p_next;
	}
	pthread_mutex_unlock(&inp_self->lock);
	return p_rval;
}
  
int
strhash_delete(strhash_pt inp_self, str_pt inp_strkey) //const char *inp_key)
{
	int bin;
	int hash = str_get_hash(inp_strkey); //hash_func(inp_key, strlen(inp_key));
	int search_key_len = str_get_len(inp_strkey);
	const char *p_search_key = str_get(inp_strkey);
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return -1;
	}
	bin = hash & inp_self->bin_mask;
	strhash_bin_pt p_prev = NULL, p_bin = inp_self->p_bins[bin];
	while(p_bin) {
		const char *p_found_key = str_get(p_bin->p_key);
		if(p_bin->hash == hash && strncmp(p_search_key, p_found_key, search_key_len) == 0) {
			if(p_prev) {
				p_prev->p_next = p_bin->p_next;
			}
			else {
				inp_self->p_bins[bin] = p_bin->p_next;
			}
			strhash_bin_free(p_bin, inp_self->p_bin_void_free_fn);
			inp_self->num_of_entries--;
			pthread_mutex_unlock(&inp_self->lock);
			return 0;
		}
		p_prev = p_bin;
		p_bin = p_bin->p_next;
	}
	pthread_mutex_unlock(&inp_self->lock);
	return 1;
}

size_t
strhash_count(strhash_pt inp_self)
{
	size_t rval;
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return -1;
	}
	rval = inp_self->num_of_entries;
	pthread_mutex_unlock(&inp_self->lock);
	return rval;
}
  

#ifdef __cplusplus
}
#endif

