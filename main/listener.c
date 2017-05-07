
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "llist.h"
#include "listener.h"

#include <event2/bufferevent.h>
#include <event2/buffer.h>

static void
echo_read_cb(struct bufferevent *bev, void *ctx)
{
        /* This callback is invoked when there is data to read on bev. */
        struct evbuffer *input = bufferevent_get_input(bev);
        struct evbuffer *output = bufferevent_get_output(bev);

        /* Copy all the data from the input buffer to the output buffer. */
        evbuffer_add_buffer(output, input);
}

static void
echo_event_cb(struct bufferevent *bev, short events, void *ctx)
{
        if (events & BEV_EVENT_ERROR)
                perror("Error from bufferevent");
        if (events & (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
                bufferevent_free(bev);
        }
}

static void
accept_conn_cb(struct evconnlistener *listener,
    evutil_socket_t fd, struct sockaddr *address, int socklen,
    void *ctx)
{
        /* We got a new connection! Set up a bufferevent for it. */
        struct event_base *base = evconnlistener_get_base(listener);
        struct bufferevent *bev = bufferevent_socket_new(
                base, fd, BEV_OPT_CLOSE_ON_FREE);

        bufferevent_setcb(bev, echo_read_cb, NULL, echo_event_cb, NULL);

        bufferevent_enable(bev, EV_READ|EV_WRITE);
}

static void
accept_error_cb(struct evconnlistener *listener, void *ctx)
{
        struct event_base *base = evconnlistener_get_base(listener);
        int err = EVUTIL_SOCKET_ERROR();
        fprintf(stderr, "Got an error %d (%s) on the listener. "
                "Shutting down.\n", err, evutil_socket_error_to_string(err));

        event_base_loopexit(base, NULL);
}

int main(int argc, char** argv) 
{
	struct event_base *p_base;
	listener_pt  p_listener;

	p_base = event_base_new();

	p_listener = listener_ctor(p_base, accept_conn_cb, accept_error_cb);
	if(p_listener) {
		if(0 != listener_set_ipaddr(p_listener, "172.17.0.2:8080")) {
			printf("p_listener set_addr failed\n");
			return 0;
		}
		if(listener_bind(p_listener) != 0) {
			printf("p_listener bind failed\n");
			return 0;
		}
	
		while(1) {
			event_base_dispatch(p_base);
		}
	}
	else {
		printf("p_listener is null\n");
	}

	listener_dtor(&p_listener);

	printf("Hello World\n");
	return 0;	
}

