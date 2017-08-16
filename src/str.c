
#include <stdlib.h>
#include <string.h>

#include "str.h"

#ifdef __cplusplus
extern C {
#endif

struct _str
{
	int		refcount;
	int		hash;
	int		len;
	const char	*str;
};

static inline int 
hash_func(const char *inp_key, int in_len)
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

str_pt
str_ctor(const char *inp, int in_len)
{
	str_pt p_self = calloc(1, sizeof(str_t));
	if(p_self) {
		p_self->str = calloc(1, in_len + 1);
		if(!p_self->str) {
			free(p_self);
			return NULL;
		}
		memcpy((void*)p_self->str, inp, in_len);
		p_self->hash = hash_func(inp, in_len);
		p_self->refcount = 1;
		p_self->len = in_len;
	}
	return p_self;
}

void
str_free(str_pt inp_self)
{
	if(inp_self) {
		__sync_fetch_and_sub(&inp_self->refcount, 1);
		if(inp_self->refcount > 0) return;
		if(inp_self->str) free((void*)inp_self->str);
		free(inp_self);
	}
}

const char*
str_get(str_pt inp_self) 
{
	const char *p_rval = NULL;
	if(inp_self) {
		p_rval = inp_self->str;
	}
	return p_rval;
}

const char*
str_get_with_len(str_pt inp_self, int *outp_len) 
{
	const char *p_rval = NULL;
	if(inp_self) {
		if(outp_len) *outp_len = inp_self->len;
		p_rval = inp_self->str;
	}
	return p_rval;
}

str_pt
str_copy_byref(str_pt inp_self)
{
	if(inp_self) {
		__sync_fetch_and_add(&inp_self->refcount, 1);
	}
	return inp_self;
}

str_pt
str_dup(str_pt inp_self)
{
	str_pt p_dup = NULL;
	if(inp_self) {
		p_dup = str_ctor(inp_self->str, inp_self->len);
	}
	return p_dup;
}

str_pt
str_concat(str_pt inp_self, str_pt inp_other)
{
	if(inp_self && inp_other && inp_other->len > 0) {
		int new_len = inp_self->len + inp_other->len;
		inp_self->str = realloc((void*)inp_self->str, new_len + 1);
		memcpy((void*)&inp_self->str[inp_self->len], inp_other->str, inp_other->len);
		inp_self->len = new_len;
		inp_self->hash = hash_func(inp_self->str, inp_self->len);
		*((char*)(inp_self->str + new_len)) = '\0';
	}
	return inp_self;
}

int
str_get_len(str_pt inp_self)
{
	int rval = -1;
	if(inp_self) {
		return inp_self->len;
	}
	return -1;
}

int
str_get_refcount(str_pt inp_self)
{
	if(inp_self) {
		return inp_self->refcount;
	}
	return -1;
}

int
str_get_hash(str_pt inp_self)
{
	if(inp_self) {
		return inp_self->hash;
	}
	return 0;
}

#ifdef __cplusplus
}
#endif

