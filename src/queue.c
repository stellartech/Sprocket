
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
  
#include "queue.h"
  
#ifdef  __cplusplus
extern "C" {
#endif
  
struct _queue_item
{
	int                 type;
	void*               p_payload;
	int                 payload_len;
	pfn_queue_item_free pfn_free;
	queue_item_pt       p_next;
};
  
struct _queue
{
	int             counter;
	queue_item_pt   p_head;
	queue_item_pt   p_tail;
	pthread_mutex_t lock;
};
  
queue_pt
queue_ctor()
{
	queue_pt p_self = (queue_pt)calloc(1, sizeof(queue_t));
	if(p_self) {
		pthread_mutex_init(&p_self->lock, NULL);
	}
	return p_self;
}
  
void
queue_dtor(queue_pt *inpp)
{
	if(inpp) {
		queue_pt p_self = *inpp;
		queue_item_pt p_next = p_self->p_head;
		while(p_next) {
			queue_item_pt p_temp = p_next->p_next;
			queue_item_dtor(&p_next);
			p_next = p_temp;
		}
		free(p_self);
		*inpp = NULL;
	}
}
  
void
queue_free(void* inp)
{
	if(inp) {
		queue_pt p_self = (queue_pt)inp;
		queue_dtor(&p_self);
	}
}
  
int
queue_size(queue_pt inp_self)
{
	int size = -1;
	if(pthread_mutex_trylock(&inp_self->lock) == 0) {
		size = inp_self->counter;
		pthread_mutex_unlock(&inp_self->lock);
	}
	return size;
}
  
queue_item_pt
queue_pop_front(queue_pt inp_self)
{
	queue_item_pt p_item = NULL;
	if(pthread_mutex_trylock(&inp_self->lock) == 0) {
		if(inp_self->p_head) {
			p_item = inp_self->p_head;
			inp_self->p_head = p_item->p_next;
			p_item->p_next = NULL;
			--inp_self->counter;
		}
		pthread_mutex_unlock(&inp_self->lock);
	}
	return p_item;
}
  
inline static void
queue_push_back_helper(queue_pt inp_self, queue_item_pt inp_item)
{
	inp_item->p_next = NULL;
	if(inp_self->p_head == NULL) {
		inp_self->p_head = inp_item;
		inp_self->p_tail = inp_item;
	}
	else {
		inp_self->p_tail->p_next = inp_item;
		inp_self->p_tail = inp_item;
	}    
	++inp_self->counter;
}
  
int
queue_push_back(queue_pt inp_self, queue_item_pt inp_item)
{
	if(pthread_mutex_trylock(&inp_self->lock) == 0) {
		queue_push_back_helper(inp_self, inp_item);
		pthread_mutex_unlock(&inp_self->lock);
		return 1;
	}
	return 0;
}
  
int
queue_push_back_blocking(queue_pt inp_self, queue_item_pt inp_item)
{
	if(pthread_mutex_lock(&inp_self->lock) == 0) {
		queue_push_back_helper(inp_self, inp_item);
		pthread_mutex_unlock(&inp_self->lock);
		return 1;
	}
	return 0;
}
  
queue_item_pt
queue_item_ctor(int inn_type, void* inp_payload, int inn_payload_len,
        pfn_queue_item_free inpfn_free)
{
	queue_item_pt p_self = (queue_item_pt)calloc(1, sizeof(queue_item_t));
	if(p_self) {
		p_self->type = inn_type;
		p_self->p_payload = inp_payload;
		p_self->payload_len = inn_payload_len;
		p_self->pfn_free = inpfn_free;
	}
	return p_self;
}
  
void
queue_item_free(void* inp)
{
	if(inp) {
		queue_item_pt p_self = (queue_item_pt)inp;
		if(p_self->pfn_free && p_self->p_payload) {
			(p_self->pfn_free)(p_self->p_payload);
		}
		free(inp);
	}
}
  
void
queue_item_dtor(queue_item_pt* inpp)
{
	if(inpp) {
		queue_item_pt p_self = *inpp;
		queue_item_free(p_self);
		*inpp = NULL;
	}
}
  
int
queue_item_get_type(queue_item_pt inp_self)
{
	return inp_self->type;
}
  
void*
queue_item_get_payload(queue_item_pt inp_self, int* outp_len)
{
	if(outp_len) {
		*outp_len = inp_self->payload_len;
	}
	return inp_self->p_payload;
}
  
#ifdef  __cplusplus
}
#endif

