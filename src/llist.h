
#ifndef LLIST_H_INCLUDED
#define LLIST_H_INCLUDED
  
#include <stdint.h>
  
#define LLIST_CONST_KEY(x) x,sizeof(x)-1
  
#ifdef __cplusplus
extern "C" {
#endif
  
typedef void (*llist_void_free_fn)(void*);
  
struct _llist;
typedef struct _llist    llist_t;
typedef struct _llist *  llist_pt;
  
struct _llist_iterator;
typedef struct _llist_iterator    llist_iterator_t;
typedef struct _llist_iterator *  llist_iterator_pt;
  
typedef void   (*_llist_free)(llist_pt inp_self);
typedef void   (*_llist_dtor)(llist_pt *inpp_self);
typedef void * (*_llist_findh)(llist_pt inp_self, uint32_t inn_hash, const char *inp_key);
typedef void * (*_llist_findl)(llist_pt inp_self, const char *inp_key, int inn_len);
typedef void * (*_llist_find)(llist_pt inp_self, const char *inp_key);
typedef int    (*_llist_insertl)(llist_pt inp_self, const char *inp_key, int inn_len, void *inp_val);
typedef int    (*_llist_insert)(llist_pt inp_self, const char *inp_key, void *inp_val);
typedef void * (*_llist_removel)(llist_pt inp_self, const char *inp_key, int inn_len);
typedef void * (*_llist_remove)(llist_pt inp_self, const char *inp_key);
typedef void   (*_llist_deletel)(llist_pt inp_self, const char *inp_key, int inn_len);
typedef void   (*_llist_delete)(llist_pt inp_self, const char *inp_key);
typedef int    (*_llist_existsl)(llist_pt inp_self, const char *inp_key, int inn_len);
typedef int    (*_llist_exists)(llist_pt inp_self, const char *inp_key);
typedef int    (*_llist_count)(llist_pt inp_self);
typedef uint32_t (*_llist_hashof)(const char *, int);
typedef int    (*_llist_iterator_forward)(llist_iterator_pt inp_self);
typedef const char*  (*_llist_iterator_key)(llist_iterator_pt inp_self);
typedef void*  (*_llist_iterator_current)(llist_iterator_pt inp_self);
typedef void   (*_llist_iterator_free)(llist_iterator_pt inp_self);
typedef const char*  (*_llist_iterator_key)(llist_iterator_pt inp_self);
typedef llist_iterator_pt (*_llist_iterator_new)(llist_pt inp_self);
  
typedef uint32_t (*_llist_hashof)(const char *, int);
  
struct _llist_if
{
    _llist_free         free;
    _llist_dtor         dtor;
    _llist_findh        findh;
    _llist_findl        findl;
    _llist_find         find;
    _llist_insertl      insertl;
    _llist_insert       insert;
    _llist_removel      removel;
    _llist_remove       remove;
    _llist_deletel      deletel;
    _llist_delete       delete;
    _llist_count        count;
    _llist_hashof       hashof;
    _llist_existsl      existsl;
    _llist_exists       exists;
    _llist_iterator_new iterator_new;
    _llist_iterator_key iterator_key;
    _llist_iterator_free iterator_free;
    _llist_iterator_forward iterator_forward;
    _llist_iterator_current iterator_current;
  
};
typedef struct _llist_if   llist_if;
typedef struct _llist_if * llist_pif;
  
llist_pif
llist_get_if(llist_pt inp_self);
  
llist_pt
llist_set_if(llist_pt inp_self, llist_pif inp_if);
  
#define LLIST_IF(x) llist_get_if(x)
  
llist_pt
llist_ctor(llist_void_free_fn in_free_fn);
  
#ifdef __cplusplus
}
#endif
  
#endif /* LLIST_H_INCLUDED */

