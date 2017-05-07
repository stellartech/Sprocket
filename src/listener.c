 

#include <stdlib.h>
#include <string.h>

#include "listener.h"

struct _listener
{
	int backlog;	// Passed to the listen() call to determine the length of the
			// acceptable connection backlog.  Set to -1 for a reasonable default.
	unsigned flags; // Any number of LEV_OPT_* flags

	short port;
	int socklen;
	struct sockaddr_in sin;
	struct event_base *p_event_base;
	struct evconnlistener *p_evconnlistener;

	evconnlistener_cb p_accept_cb;
	evconnlistener_errorcb p_error_cb;

};

int
listener_bind(listener_pt inp_self)
{
	if(inp_self) {
		if(inp_self->p_evconnlistener) {
			evconnlistener_free(inp_self->p_evconnlistener);
		}
		inp_self->p_evconnlistener = evconnlistener_new_bind(
			inp_self->p_event_base, 
			inp_self->p_accept_cb, (void*)inp_self,
			inp_self->flags, 
			inp_self->backlog,
			(struct sockaddr*)&inp_self->sin, 
			sizeof(struct sockaddr)); 
		if(inp_self->p_evconnlistener == NULL) {
			return -1;
		}
		else if(inp_self->p_error_cb)  {
			evconnlistener_set_error_cb(inp_self->p_evconnlistener, 
				inp_self->p_error_cb);
		}
	}
	return 0;
}


listener_pt
listener_ctor(struct event_base *inp_event_base, 
	evconnlistener_cb inp_accept_cb,
	evconnlistener_errorcb inp_error_cb)
{
	listener_pt p_self = calloc(1, sizeof(listener_t));
	if(p_self) {
		p_self->p_accept_cb = inp_accept_cb;
		p_self->p_error_cb = inp_error_cb;
		p_self->p_event_base = inp_event_base;
		p_self->backlog = -1; // Default value.
		p_self->flags = LEV_OPT_CLOSE_ON_FREE|LEV_OPT_REUSEABLE;
	}
	return p_self;
}

void
listener_free(listener_pt inp_self)
{
	if(inp_self) {
		if(inp_self->p_evconnlistener) {
			evconnlistener_free(inp_self->p_evconnlistener);
		}
		free(inp_self);
	}	
}

int
listener_pause(listener_pt inp_self)
{
	if(inp_self && inp_self->p_evconnlistener) {
		return evconnlistener_disable(inp_self->p_evconnlistener);
	}
	return -1;
}

int
listener_resume(listener_pt inp_self)
{
	if(inp_self && inp_self->p_evconnlistener) {
		return evconnlistener_enable(inp_self->p_evconnlistener);
	}
	return -1;
}


void
listener_dtor(listener_pt *inpp_self)
{
	if(inpp_self) {
		listener_pt p_self = *inpp_self;
		if(p_self) {
			listener_free(p_self);
		}
		*inpp_self = NULL;
	}
}


listener_pt
listener_set_backlog(listener_pt inp_self, int in_backlog)
{
	if(inp_self) {
		inp_self->backlog = in_backlog;
	}
	return inp_self;
}

listener_pt
listener_set_flags(listener_pt inp_self, unsigned in_flags)
{
	if(inp_self) {
		inp_self->flags = in_flags;
	}
	return inp_self;
}

int
listener_set_ipaddr(listener_pt inp_self, const char *inp_addr)
{
	if(inp_self) {
		char buf[256];
		int len = sizeof(struct sockaddr_in);
		memset(&inp_self->sin, 0, sizeof(struct sockaddr_in));
		return evutil_parse_sockaddr_port(inp_addr, 
			(struct sockaddr*)&inp_self->sin, &len);
	}	
	return -2;
}


