#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED
  
#ifdef  __cplusplus
extern "C" {
#endif
  
typedef void (*pfn_queue_item_free)(void*);
     
struct _queue;
typedef struct _queue   queue_t;
typedef struct _queue * queue_pt;
  
struct _queue_item;
typedef struct _queue_item   queue_item_t;
typedef struct _queue_item * queue_item_pt;
  
queue_pt
queue_ctor();
  
void
queue_dtor(queue_pt *inpp);
  
void
queue_free(void* inp);
  
queue_item_pt
queue_pop_front(queue_pt inp_self);
  
int
queue_push_back(queue_pt inp_self, queue_item_pt inp_item);
  
int
queue_push_back_blocking(queue_pt inp_self, queue_item_pt inp_item);
  
int
queue_size(queue_pt inp_self);
  
queue_item_pt
queue_item_ctor(int inn_type, void* inp_payload, int inn_payload_len,
        pfn_queue_item_free inpfn_free);
  
void
queue_item_free(void* inp);
  
void
queue_item_dtor(queue_item_pt* inpp);
  
int
queue_item_get_type(queue_item_pt inp_self);
  
void*
queue_item_get_payload(queue_item_pt inp_self, int* outp_len);
  
#ifdef  __cplusplus
}
#endif
  
#endif  /* QUEUE_H_INCLUDED */

