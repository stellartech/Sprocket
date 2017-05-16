





#ifndef REACTOR_H_INCLUDED
#define REACTOR_H_INCLUDED

#include <stdint.h>

#ifdef __cplusplus
extern C {
#endif

typedef enum 
{
	REACTOR_IN = 0x001,
	REACTOR_PRI = 0x002,
	REACTOR_OUT = 0x004,
	REACTOR_RDNORM = 0x040,
	REACTOR_RDBAND = 0x080,
	REACTOR_WRNORM = 0x100,
	REACTOR_WRBAND = 0x200,
	REACTOR_MSG = 0x400,
	REACTOR_ERR = 0x008,
	REACTOR_HUP = 0x010,
	REACTOR_RDHUP = 0x2000,
	REACTOR_WAKEUP = 1u << 29,
	REACTOR_ONESHOT = 1u << 30,
	REACTOR_ET = 1u << 31
}
reactor_events_e;

struct _reactor;
typedef struct _reactor   reactor_t;
typedef struct _reactor * reactor_pt;

typedef struct
{
	int	errornumber;
	int	listener_fd;
	int	accecpt_fd;
	void	*p_userdata;
	socklen_t in_len;
	const struct sockaddr *p_addr;
}
reactor_cb_accept_args_t, *reactor_cb_accept_args_pt;

typedef struct
{
	reactor_events_e event;
	void *p_userdata;
}
reactor_cb_event_args_t, *reactor_cb_event_args_pt;

typedef struct
{
	int  listening_fd;
	void *p_userdata;
}
reactor_cb_error_args_t, *reactor_cb_error_args_pt;

typedef enum
{
	REACTOR_ACCEPT = 0,
	REACTOR_EVENT  = 1,
	REACTOR_ERROR  = 2
}
reactor_event_cb_types_e;

typedef struct
{
	reactor_event_cb_types_e type;
	union {
		reactor_cb_accept_args_t accept_args;
		reactor_cb_event_args_t  event_args;
		reactor_cb_error_args_t  error_args;
	} data;
}
reactor_cb_args_t, *reactor_cb_args_pt;

typedef int (*reactor_cb)(const reactor_cb_args_pt inp_atgs);

typedef struct
{
	int			listener_fd;
	uint32_t		event_flags;
	reactor_cb		p_callback;
	void			*p_userdata;
}
reactor_ctor_args_t, *reactor_ctor_args_pt;

reactor_pt
reactor_ctor(reactor_ctor_args_pt inp_args);

void
reactor_free(void *inp);

void
reactor_dtor(reactor_pt *inpp);

int
reactor_loop_once_for(reactor_pt inp_self, int in_timeout);

int
reactor_loop_once(reactor_pt inp_self);

int
reactor_react_to(reactor_pt inp_self, int listener_fd);

int
reactor_unreact_to(reactor_pt inp_self, int listener_fd);

reactor_pt
reactor_set_userdata(reactor_pt inp_self, void *inp);

reactor_pt
reactor_set_cb(reactor_pt inp_self, reactor_cb inp_cb);

reactor_pt
reactor_set_event_flags(reactor_pt inp_self, uint32_t in_flags);

#ifdef __cplusplus
}
#endif

#endif /* REACTOR_H_INCLUDED */


