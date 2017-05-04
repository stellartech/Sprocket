

#ifndef THD_WORKER_H_INCLUDED
#define THD_WORKER_H_INCLUDED

#ifdef __cplusplus
extern C {
#endif

typedef void (*thdfunc_pt)(void*);

typedef void (*thd_free_msg_args_pt)(void*);

struct _thd_wkr_msg;
typedef struct _thd_wkr_msg   thd_wkr_msg_t;
typedef struct _thd_wkr_msg * thd_wkr_msg_pt; 

thd_wkr_msg_pt
thd_wkr_msg_ctor(thdfunc_pt inp_func, void *inp_args, thd_free_msg_args_pt inp_free);

void
thd_wkr_msg_free(thd_wkr_msg_pt inp_self);

void
thd_wkr_msg_dtor(thd_wkr_msg_pt *inpp_self);

thdfunc_pt
thd_wkr_msg_get_func(thd_wkr_msg_pt inp_self);

void*
thd_wkr_msg_get_args(thd_wkr_msg_pt inp_self);

///////////

struct _thd_wkr_queue;
typedef struct _thd_wkr_queue   thd_wkr_queue_t;
typedef struct _thd_wkr_queue * thd_wkr_queue_pt;

thd_wkr_queue_pt
thd_wkr_queue_ctor(void);

void
thd_wkr_queue_free(thd_wkr_queue_pt inp_self);

void
thd_wkr_queue_dtor(thd_wkr_queue_pt *inpp_self);

#define THD_WKR_QUEUE_SUCCESS 0
#define THD_WKR_QUEUE_LOCKFAIL 1
#define THD_WKR_QUEUE_INVALID -1

typedef enum thd_wkr_queue_rval {
	e_thd_wkr_queue_rval_success = 0,
	e_thd_wkr_queue_rval_lockfail = 1,
	e_thd_wkr_queue_rval_invalid = -1
} e_thd_wkr_queue_rval;

e_thd_wkr_queue_rval
thd_wkr_queue_push_back(thd_wkr_queue_pt inp_self, thd_wkr_msg_pt inp_msg);

e_thd_wkr_queue_rval
thd_wkr_queue_push_back_blocking(thd_wkr_queue_pt inp_self, thd_wkr_msg_pt inp_msg);

thd_wkr_msg_pt
thd_wkr_queue_pop(thd_wkr_queue_pt inp_self);

thd_wkr_msg_pt
thd_wkr_queue_pop_blocking(thd_wkr_queue_pt inp_self);

int
thd_wkr_queue_size(thd_wkr_queue_pt inp_self);

size_t
thd_wkr_queue_sizeof(void);

//////

struct _thd_wkr;
typedef struct _thd_wkr   thd_wkr_t;
typedef struct _thd_wkr * thd_wkr_pt;

thd_wkr_pt
thd_wkr_ctor(thd_wkr_queue_pt inp_parent_in, thd_wkr_queue_pt inp_parent_out);

void
thd_wkr_free(thd_wkr_pt inp_self);

void
thd_wkr_dtor(thd_wkr_pt *inpp_self);

size_t
thd_wkr_sizeof(void);

#ifdef __cplusplus
}
#endif

#endif /* THD_WORKER_H_INCLUDED */
