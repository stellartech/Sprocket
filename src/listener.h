 
#ifndef WS_LISTENER_H_INCLUDED
#define WS_LISTENER_H_INCLUDED

#include <arpa/inet.h>
#include <event2/event.h>
#include <event2/listener.h>

struct _listener;
typedef struct _listener   listener_t;
typedef struct _listener * listener_pt;


listener_pt
listener_ctor(struct event_base *inp,
	evconnlistener_cb inp_accept_cb,
        evconnlistener_errorcb inp_error_cb);

void
listener_free(listener_pt inp_self);

void
listener_dtor(listener_pt *inpp_self);

listener_pt
listener_set_backlog(listener_pt inp_self, int in_backlog);

listener_pt
listener_set_flags(listener_pt inp_self, unsigned in_flags);

int
listener_set_ipaddr(listener_pt inp_self, const char *inp_addr);

int
listener_bind(listener_pt inp_self);

int
listener_pause(listener_pt inp_self);

int
listener_resume(listener_pt inp_self);

#endif /* WS_LISTENER_H_INCLUDED */

