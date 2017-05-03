
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
  
#include "llist.h"
  
#ifdef __cplusplus
extern "C" {
#endif
  
static inline uint32_t hash_func(const char *inp_key, uint32_t in_len);
static void llist_std_if(llist_pt inp_self);
  
struct _llist_entry;
typedef struct _llist_entry    llist_entry_t;
typedef struct _llist_entry *  llist_entry_pt;
typedef struct _llist_entry ** llist_entry_ppt;
  
struct _llist_entry
{
    uint32_t        hash;
    const char*     p_key;
    void*           p_value;
    llist_entry_pt  p_next;
};
  
struct _llist
{
    llist_pif           p_if;
    int                 counter;
    llist_entry_pt      p_head;
    llist_void_free_fn  p_entry_void_free_fn;
};
  
struct _llist_iterator
{
    llist_pt        p_list;
    llist_entry_pt  p_current;
};
  
  
llist_pt
llist_ctor(llist_void_free_fn in_free_fn)
{
    llist_pt p_self = (llist_pt)calloc(1, sizeof(llist_t));
    if(p_self) {
        p_self->p_entry_void_free_fn = in_free_fn;
        p_self->p_head = NULL;
        p_self->counter = 0;
        p_self->p_if = calloc(1, sizeof(llist_if));
        if(!p_self->p_if) {
            free(p_self);
            p_self = NULL;
        }
        else {
            llist_std_if(p_self);
        }
    }
    return p_self;
}
  
llist_pif
llist_get_if(llist_pt inp_self)
{
    if(inp_self) {
        return inp_self->p_if;
    }
    return NULL;
}
  
llist_pt
llist_set_if(llist_pt inp_self, llist_pif inp_if)
{
    if(inp_self->p_if) {
        free(inp_self->p_if);
    }
    inp_self->p_if = inp_if;
    return inp_self;
}
  
static llist_entry_pt
llist_entry_ctor(const char* inp_key, int inn_len, void* inp_value)
{
    llist_entry_pt p_entry = (llist_entry_pt)calloc(1, sizeof(llist_entry_t));
    if(p_entry) {
        p_entry->hash = hash_func(inp_key, inn_len);
        p_entry->p_key = strndup(inp_key, inn_len);
        p_entry->p_value = inp_value;
        p_entry->p_next = NULL;
    }
    return p_entry;
}
  
static llist_entry_pt
llist_entry_free(llist_entry_pt inp_self, llist_void_free_fn in_fnf)
{
    llist_entry_pt p_next = NULL;
    if(inp_self) {
        p_next = inp_self->p_next;
        if(in_fnf && inp_self->p_value) {
            (in_fnf)(inp_self->p_value);
        }
        if(inp_self->p_key) {
            free((void*)inp_self->p_key);
        }
        free(inp_self);
    }
    return p_next;
}
  
static void
llist_free(llist_pt inp_self)
{
    if(inp_self) {
        llist_entry_pt p_entry = inp_self->p_head;
        while(p_entry) {
            llist_entry_pt p_next = p_entry->p_next;
            llist_entry_free(p_entry, inp_self->p_entry_void_free_fn);
            p_entry = p_next;
        }
        free(inp_self->p_if);
        free(inp_self);
    }
}
  
static void
llist_dtor(llist_pt *inpp)
{
    if(inpp) {
        llist_pt p_self = *inpp;
        if(p_self) {
            llist_free(p_self);
        }
        *inpp = NULL;
    }
}
  
static void*
llist_findh(llist_pt inp_self, uint32_t inn_hash, const char *inp_key)
{
    llist_entry_pt p_entry = inp_self->p_head;
    while(p_entry) {
        if(p_entry->hash == inn_hash) {
            if(!strcmp(p_entry->p_key, inp_key)) {
                return p_entry->p_value;
            }
        }
        p_entry = p_entry->p_next;
    }
    return NULL;
}
  
static void*
llist_findl(llist_pt inp_self, const char *inp_key, int inn_len)
{
    uint32_t hash = hash_func(inp_key, inn_len);
    return llist_findh(inp_self, hash, inp_key);
}
  
static void*
llist_find(llist_pt inp_self, const char *inp_key)
{
    return llist_findl(inp_self, inp_key, strlen(inp_key));
}
  
static int
llist_insertl(llist_pt inp_self, const char *inp_key, int inn_len, void *inp_val)
{
    llist_entry_pt p_entry = llist_entry_ctor(inp_key, inn_len, inp_val);
    if(p_entry) {
        p_entry->p_next = inp_self->p_head;
        inp_self->p_head = p_entry;
        ++inp_self->counter;
        return 1;
    }
    return 0;
}
  
static int
llist_insert(llist_pt inp_self, const char *inp_key, void *inp_val)
{
    return llist_insertl(inp_self, inp_key, strlen(inp_key), inp_val);
}
  
static void*
llist_removeh(llist_pt inp_self, uint32_t inn_hash, const char *inp_key)
{
    llist_entry_pt p_entry = inp_self->p_head;
    if(inp_self->p_head && inp_self->p_head->hash == inn_hash &&
        !strcmp(inp_self->p_head->p_key, inp_key)) 
    {
        inp_self->p_head = inp_self->p_head->p_next;
        void* p_rval = p_entry->p_value;
        free((void*)p_entry->p_key);
        free(p_entry);
        --inp_self->counter;
        return p_rval;
    }
    else {
        llist_entry_pt p_prev = NULL;
        while(p_entry) {
            if(p_entry->hash == inn_hash && !strcmp(p_entry->p_key, inp_key)) {
                void* p_rval = p_entry->p_value;
                if(p_prev) {
                    p_prev->p_next = p_entry->p_next;
                }
                free((void*)p_entry->p_key);
                free(p_entry);
                --inp_self->counter;
                return p_rval;
            }
            p_prev = p_entry;
            p_entry = p_entry->p_next;
        }
    }
    return NULL;
}
  
static void*
llist_removel(llist_pt inp_self, const char *inp_key, int inn_len)
{
    uint32_t hash = hash_func(inp_key, inn_len);
    return llist_removeh(inp_self, hash, inp_key);
}
  
static void*
llist_remove(llist_pt inp_self, const char *inp_key)
{
    return llist_removel(inp_self, inp_key, strlen(inp_key));
}
  
static void
llist_deletel(llist_pt inp_self, const char *inp_key, int inn_len)
{
    void* p_val = llist_removel(inp_self, inp_key, inn_len);
    if(p_val && inp_self->p_entry_void_free_fn) {
        (inp_self->p_entry_void_free_fn)(p_val);
    }
}
  
static void
llist_delete(llist_pt inp_self, const char *inp_key)
{
    llist_deletel(inp_self, inp_key, strlen(inp_key));
}
  
static int
llist_existsl(llist_pt inp_self, const char *inp_key, int inn_len)
{
    uint32_t hash = hash_func(inp_key, inn_len);
    llist_entry_pt p_entry = inp_self->p_head;
    while(p_entry) {
        if(hash == p_entry->hash && !strcmp(inp_key, p_entry->p_key)) {
            return 1;
        }
        p_entry = p_entry->p_next;
    }
    return 0;
}
  
static int
llist_exists(llist_pt inp_self, const char *inp_key)
{
    return llist_existsl(inp_self, inp_key, strlen(inp_key));
}
  
static int
llist_count(llist_pt inp_self)
{
    return inp_self->counter;
}
  
static uint32_t
llist_hashl(const char *inp_key, int inn_len)
{
    return hash_func(inp_key, inn_len);
}
  
static uint32_t
llist_hash(const char *inp_key)
{
    return hash_func(inp_key, strlen(inp_key));
}
  
static int
llist_iterator_forward(llist_iterator_pt inp_self)
{
    if(inp_self->p_current && inp_self->p_current->p_next) {
        inp_self->p_current = inp_self->p_current->p_next;
        return 1;
    }
    return 0;
}
  
static void*  
llist_iterator_current(llist_iterator_pt inp_self)
{
    if(inp_self->p_current) {
        return inp_self->p_current->p_value;
    }
    return NULL;
}
  
static void  
llist_iterator_free(llist_iterator_pt inp_self)
{
    if(inp_self) {
        free(inp_self);
    }
}
  
static const char*  
llist_iterator_key(llist_iterator_pt inp_self) 
{
    if(inp_self->p_current) {
        return inp_self->p_current->p_key;
    }
    return NULL;
}
  
static llist_iterator_pt 
llist_iterator_new(llist_pt inp_self)
{
    llist_iterator_pt p_self = NULL;
    if(inp_self) {
        p_self = calloc(1, sizeof(llist_iterator_t));
        if(p_self) {
            p_self->p_list = inp_self;
            p_self->p_current = inp_self->p_head;
        }
    }
    return p_self;
}
  
static inline uint32_t
hash_func(const char *inp_key, uint32_t in_len) // djb2 hash
{
    register uint32_t hash = 5381;
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
        case 7: hash = ((hash << 5) + hash) + *inp_key++; /* fallthro */
        case 6: hash = ((hash << 5) + hash) + *inp_key++; /* fallthro */
        case 5: hash = ((hash << 5) + hash) + *inp_key++; /* fallthro */
        case 4: hash = ((hash << 5) + hash) + *inp_key++; /* fallthro */
        case 3: hash = ((hash << 5) + hash) + *inp_key++; /* fallthro */
        case 2: hash = ((hash << 5) + hash) + *inp_key++; /* fallthro */
        case 1: hash = ((hash << 5) + hash) + *inp_key++; break;
        case 0: default: break;
    }
    return hash;
}
  
static void
llist_std_if(llist_pt inp_self)
{
    inp_self->p_if->count = llist_count;
    inp_self->p_if->delete = llist_delete;
    inp_self->p_if->deletel = llist_deletel;
    inp_self->p_if->dtor = llist_dtor;
    inp_self->p_if->find = llist_find;
    inp_self->p_if->findh = llist_findh;
    inp_self->p_if->findl = llist_findl;
    inp_self->p_if->free = llist_free;
    inp_self->p_if->insert = llist_insert;
    inp_self->p_if->insertl = llist_insertl;
    inp_self->p_if->remove = llist_remove;  
    inp_self->p_if->removel = llist_removel;  
    inp_self->p_if->exists = llist_exists;  
    inp_self->p_if->existsl = llist_existsl;  
    inp_self->p_if->hashof = llist_hashl;
    inp_self->p_if->iterator_new = llist_iterator_new;
    inp_self->p_if->iterator_key = llist_iterator_key;
    inp_self->p_if->iterator_free = llist_iterator_free;
    inp_self->p_if->iterator_current = llist_iterator_current;
    inp_self->p_if->iterator_forward = llist_iterator_forward;
}
  
#ifdef __cplusplus
}
#endif

