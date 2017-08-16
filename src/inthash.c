
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>
  
#include "inthash.h"
  
#ifdef __cplusplus
extern "C" {
#endif
  
struct _inthash_bin;
typedef struct _inthash_bin    inthash_bin_t;
typedef struct _inthash_bin *  inthash_bin_pt;
typedef struct _inthash_bin ** inthash_bin_ppt;
  
struct _inthash_bin
{
	int		n_index;
	void*           p_value;
	inthash_bin_pt  p_next;
};
  
struct _inthash
{
	size_t                num_of_entries;
	int                   num_of_bins;
	int                   bin_mask;
	inthash_void_free_fn  p_bin_void_free_fn;
	inthash_bin_ppt       p_bins;
	pthread_mutex_t       lock;
};
  
inthash_pt
inthash_ctor(int in_num_bins, inthash_void_free_fn in_free_fn)
{
	inthash_pt p_self = (inthash_pt)calloc(1, sizeof(inthash_t));
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
		p_self->p_bins = (inthash_bin_ppt)calloc(p_self->num_of_bins,
			sizeof(inthash_bin_ppt));
		if(!p_self->p_bins) {
			goto inthash_ctor_fail;
		}
		if(pthread_mutex_init(&p_self->lock, NULL) != 0) {
			goto inthash_ctor_fail;
		}
	}
	return p_self;

inthash_ctor_fail:
	if(p_self) {
		inthash_dtor(&p_self);
	}
	return NULL;
}
  
static inthash_bin_pt
inthash_bin_free(inthash_bin_pt inp_self, inthash_void_free_fn in_fnf)
{
	inthash_bin_pt p_next = NULL;
	if(inp_self) {
		p_next = inp_self->p_next;
		if(in_fnf && inp_self->p_value) {
			(in_fnf)(inp_self->p_value);
		}
		free(inp_self);
	}
	return p_next;
}
  
void
inthash_decref(inthash_pt inp_self)
{
	if(inp_self) {
		pthread_mutex_lock(&inp_self->lock);	
		for(int i = 0; i < inp_self->num_of_bins; i++) {
			inthash_bin_pt p_next = inthash_bin_free(inp_self->p_bins[i],
			   inp_self->p_bin_void_free_fn);
			inp_self->p_bins[i] = 0;
			while(p_next != NULL) {
				p_next = inthash_bin_free(p_next,
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
inthash_dtor(inthash_pt *inpp)
{
	if(inpp) {
		inthash_pt p_self = *inpp;
		if(p_self) inthash_decref(p_self);
		*inpp = NULL;
	}
}
  
void*
inthash_find(inthash_pt inp_self, int in_index) 
{
	int bin;
	void *p_rval = NULL;
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return NULL;
	}
	bin = in_index & inp_self->bin_mask;
	inthash_bin_pt p_bin = (inthash_bin_pt)inp_self->p_bins[bin];
	while(p_bin) {
		if(in_index == p_bin->n_index) {
			p_rval = p_bin->p_value;
			pthread_mutex_unlock(&inp_self->lock);
			return p_rval; 
		}
		p_bin = p_bin->p_next;
	}
	pthread_mutex_unlock(&inp_self->lock);
	return p_rval;
}
  
int
inthash_insert(inthash_pt inp_self, int in_index, void *inp_val)
{
	int bin;
	if(inthash_find(inp_self, in_index) != NULL) {
		inthash_delete(inp_self, in_index);
	}
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return -1;
	}
	inthash_bin_pt p_bin = calloc(1, sizeof(inthash_bin_t));
	if(!p_bin) {
		pthread_mutex_unlock(&inp_self->lock);
		return -1;
	}
	p_bin->p_value = inp_val;
	p_bin->n_index = in_index;
	bin = in_index & inp_self->bin_mask;
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
inthash_remove(inthash_pt inp_self, int in_index)
{
	int bin;
	void *p_rval = NULL;
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return NULL;
	}
	p_rval = NULL;
	bin = in_index & inp_self->bin_mask;
	inthash_bin_pt p_prev = NULL, p_bin = inp_self->p_bins[bin];
	while(p_bin) {
		if(in_index == p_bin->n_index) { 
			if(p_prev) {
				p_prev->p_next = p_bin->p_next;
			}
			else {
				inp_self->p_bins[bin] = p_bin->p_next;
			}
			inp_self->num_of_entries--;
			p_rval = p_bin->p_value;
			inthash_bin_free(p_bin, NULL);
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
inthash_delete(inthash_pt inp_self, int in_index)
{
	int bin;
	if(pthread_mutex_lock(&inp_self->lock) != 0) {
		return -1;
	}
	bin = in_index & inp_self->bin_mask;
	inthash_bin_pt p_prev = NULL, p_bin = inp_self->p_bins[bin];
	while(p_bin) {
		if(in_index == p_bin->n_index) { 
			if(p_prev) {
				p_prev->p_next = p_bin->p_next;
			}
			else {
				inp_self->p_bins[bin] = p_bin->p_next;
			}
			inthash_bin_free(p_bin, inp_self->p_bin_void_free_fn);
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
inthash_count(inthash_pt inp_self)
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

