


#include <stdlib.h>
#include <pthread.h>

#include "thd_worker.h"

#ifdef __cplusplus
extern C {
#endif

struct _thd_wkr_msg
{
	thdfunc_pt           p_func;
	void                *p_args;
	thd_free_msg_args_pt p_args_free;
	struct _thd_wkr_msg *p_next;
};

thd_wkr_msg_pt
thd_wkr_msg_ctor(thdfunc_pt inp_func, void *inp_args, thd_free_msg_args_pt inp_free)
{
	thd_wkr_msg_pt p_self = calloc(1, sizeof(thd_wkr_msg_t));
	if(p_self) {
		p_self->p_func      = inp_func;
		p_self->p_args      = inp_args;
		p_self->p_args_free = inp_free;
		p_self->p_next      = NULL;
	}
	return p_self;
}

void
thd_wkr_msg_free(thd_wkr_msg_pt inp_self)
{
	if(inp_self) {
		if(inp_self->p_args && inp_self->p_args_free) {
			(inp_self->p_args_free)(inp_self->p_args);
		}
		free(inp_self);
	}
}

void
thd_wkr_msg_dtor(thd_wkr_msg_pt *inpp_self)
{
	if(inpp_self) {
		thd_wkr_msg_pt p_self = *inpp_self;
		thd_wkr_msg_free(p_self);
		*inpp_self = NULL;
	}
}

thdfunc_pt
thd_wkr_msg_get_func(thd_wkr_msg_pt inp_self)
{
	return inp_self ? inp_self->p_func : NULL;
}

void*
thd_wkr_msg_get_args(thd_wkr_msg_pt inp_self)
{
	return inp_self ? inp_self->p_args : NULL;
}

///////////

struct _thd_wkr_queue 
{
	int counter;
	thd_wkr_msg_pt	p_head;
	thd_wkr_msg_pt p_tail;
	pthread_mutex_t lock;
};

thd_wkr_queue_pt
thd_wkr_queue_ctor(void)
{
	thd_wkr_queue_pt p_self = calloc(1, sizeof(thd_wkr_queue_t));
	if(p_self) {
		if(pthread_mutex_init(&p_self->lock, NULL) != 0) {
			free(p_self);
			p_self = NULL;
		}
	}
	return p_self;
}

void
thd_wkr_queue_free(thd_wkr_queue_pt inp_self)
{
	if(inp_self) {
		pthread_mutex_destroy(&inp_self->lock);
	}
	free(inp_self);
}

void
thd_wkr_queue_dtor(thd_wkr_queue_pt *inpp_self)
{
	if(inpp_self) {
		thd_wkr_queue_pt p_self = *inpp_self;
		thd_wkr_queue_free(p_self);
		*inpp_self = NULL;
	}
}

inline static void
thd_wkr_queue_push_back_helper(thd_wkr_queue_pt inp_self, thd_wkr_msg_pt inp_msg)
{
    inp_msg->p_next = NULL;
    if(inp_self->p_head == NULL) {
        inp_self->p_head = inp_msg;
        inp_self->p_tail = inp_msg;
    }
    else {
        inp_self->p_tail->p_next = inp_msg;
        inp_self->p_tail = inp_msg;
    }    
    ++inp_self->counter;
}

e_thd_wkr_queue_rval
thd_wkr_queue_push_back(thd_wkr_queue_pt inp_self, thd_wkr_msg_pt inp_msg)
{
	if(inp_self) {
		if(pthread_mutex_trylock(&inp_self->lock) == 0) {
			thd_wkr_queue_push_back_helper(inp_self, inp_msg);
			pthread_mutex_unlock(&inp_self->lock);
			return e_thd_wkr_queue_rval_success;
		}
		else {
			return e_thd_wkr_queue_rval_lockfail;
		}
	}
	return e_thd_wkr_queue_rval_invalid;	
}

e_thd_wkr_queue_rval
thd_wkr_queue_push_back_blocking(thd_wkr_queue_pt inp_self, thd_wkr_msg_pt inp_msg)
{
	if(inp_self) {
		if(pthread_mutex_lock(&inp_self->lock) == 0) {
			thd_wkr_queue_push_back_helper(inp_self, inp_msg);
			pthread_mutex_unlock(&inp_self->lock);
			return e_thd_wkr_queue_rval_success;
		}
		else {
			return e_thd_wkr_queue_rval_lockfail;
		}
	}
	return e_thd_wkr_queue_rval_invalid;	
}

thd_wkr_msg_pt
thd_wkr_queue_pop(thd_wkr_queue_pt inp_self)
{
	thd_wkr_msg_pt p_rval = NULL;
	if(inp_self && pthread_mutex_trylock(&inp_self->lock) == 0) {
		if(inp_self->p_head) {
			p_rval = inp_self->p_head;
			inp_self->p_head = p_rval->p_next;
			p_rval->p_next = NULL;
			--inp_self->counter;
		}
		pthread_mutex_unlock(&inp_self->lock);
	}
	return p_rval;
}

thd_wkr_msg_pt
thd_wkr_queue_pop_blocking(thd_wkr_queue_pt inp_self)
{
	thd_wkr_msg_pt p_rval = NULL;
	if(inp_self && pthread_mutex_lock(&inp_self->lock) == 0) {
		if(inp_self->p_head) {
			p_rval = inp_self->p_head;
			inp_self->p_head = p_rval->p_next;
			p_rval->p_next = NULL;
			--inp_self->counter;
		}
		pthread_mutex_unlock(&inp_self->lock);
	}
	return p_rval;
}


int
thd_wkr_queue_size(thd_wkr_queue_pt inp_self)
{
	return inp_self ? inp_self->counter : -1;
}

size_t
thd_wkr_queue_sizeof(void)
{
	return sizeof(thd_wkr_queue_t);
}

//////

#include <sched.h>
#include <semaphore.h>

struct _thd_wkr
{
        sem_t             sem_run_flag;
        sem_t             sem_running;
        pthread_t         pthread;
        pthread_attr_t    pthread_attr;
        thd_wkr_queue_pt  p_parent_in;
        thd_wkr_queue_pt  p_parent_out;
};

static void*
thd_wrk_run(void *inp)
{
	volatile int sem_value;
	thd_wkr_msg_pt p_msg;
	thd_wkr_pt p_self = (thd_wkr_pt)inp;

	while(1) {
		sem_value = 1;
		sem_getvalue(&p_self->sem_run_flag, (int*)&sem_value);	
		if(sem_value > 0) break;

		if((p_msg = thd_wkr_queue_pop(p_self->p_parent_in)) == NULL) {
			sched_yield();
		}
		else {
			thdfunc_pt p_func = thd_wkr_msg_get_func(p_msg);
			if(p_func) {
				void *p_args = thd_wkr_msg_get_args(p_msg);
				(p_func)(p_args);
			}
			
		}
		if(p_msg) {
			thd_wkr_msg_dtor(&p_msg);
		}
	}
	sem_post(&p_self->sem_running);
	return NULL;
}

thd_wkr_pt
thd_wkr_ctor(thd_wkr_queue_pt inp_parent_in, thd_wkr_queue_pt inp_parent_out)
{
	thd_wkr_pt p_self = calloc(1, sizeof(thd_wkr_t));
	if(p_self) {
		p_self->p_parent_in  = inp_parent_in;
		p_self->p_parent_out = inp_parent_out;
		if(pthread_attr_init(&p_self->pthread_attr) != 0) {
			goto thd_wkr_ctor_failed;
		}		 
		if(pthread_attr_setdetachstate(&p_self->pthread_attr, 
			PTHREAD_CREATE_DETACHED) != 0) {
			goto thd_wkr_ctor_failed;
		}
		sem_init(&p_self->sem_run_flag, 0, 0);
		sem_init(&p_self->sem_running, 0, 0);
		if(pthread_create(&p_self->pthread, &p_self->pthread_attr, thd_wrk_run, (void*)p_self) != 0) {
			goto thd_wkr_ctor_failed;
		}
	}
	return p_self;

thd_wkr_ctor_failed:
	if(p_self) {
		thd_wkr_dtor(&p_self);
	}
	return NULL;
}

void
thd_wkr_free(thd_wkr_pt inp_self)
{
	if(inp_self) {
		volatile int wait;
		pthread_attr_destroy(&inp_self->pthread_attr);
		sem_getvalue(&inp_self->sem_running, (int*)&wait);
		sem_post(&inp_self->sem_run_flag);
		while(wait == 0) {
			sem_getvalue(&inp_self->sem_running, (int*)&wait);
		}
		sem_destroy(&inp_self->sem_run_flag);
		sem_destroy(&inp_self->sem_running);
		free(inp_self);
	}
}

void
thd_wkr_dtor(thd_wkr_pt *inpp_self)
{
	if(inpp_self && *inpp_self) {
		thd_wkr_free(*inpp_self);
		*inpp_self = NULL;
	}
}

size_t
thd_wkr_sizeof(void)
{
	return (size_t)sizeof(thd_wkr_t);
}

#ifdef __cplusplus
}
#endif

