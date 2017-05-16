/*********************************************************************************
 *   Copyright (c) 2008-2017 Andy Kirkham  All rights reserved.
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom
 *   the Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included
 *   in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 ***********************************************************************************/

// Uses listen_t to create a listening socket and then uses
// reactor_t to react to events on that listening socket.
// A simple example of use case for these, use curl or netcat
// to test it out. This will probably form part of an integration
// test suite at some point (as opposed to unit test suits for
// code blocks).
//
// Note, the event loop only runs for 10 seconds and then the
// program terminates. This allows it to run under valgrind
// to check for memory leaks.

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "../src/reactor.h"
#include "../src/listener.h"

#define MAXEVENTS 64

typedef struct
{
	reactor_pt 	p_reactor;
	listener_pt	p_listener;
}
my_globals_t;

static int
my_callback(const reactor_cb_args_pt inp_args);

int
main(int argc, char *argv[])
{
	int loop_counter = 0;
	my_globals_t glbs;

	glbs.p_listener = listener_ctor("0.0.0.0", 8081);
	if(!glbs.p_listener) {
		printf("Failed ctor()\n");
		return 0;
	}

	if(listener_bind(glbs.p_listener) < 1) {
		printf("Failed bind\n");
		listener_dtor(&glbs.p_listener);
		return 0;
	}
	listener_set_backlog(glbs.p_listener, 1024);

	{ // Create a reactor for the listening socket
		reactor_ctor_args_t args;
		args.listener_fd = listener_get_fd(glbs.p_listener);
		args.event_flags = REACTOR_IN | REACTOR_OUT | REACTOR_ET | REACTOR_RDHUP;
		args.p_callback = my_callback;
		args.p_userdata = &glbs;
		glbs.p_reactor = reactor_ctor(&args);
	}

	// Activate the listener.
	listener_listen(glbs.p_listener);

	// Basic event loop
	while(loop_counter < 10) {
		reactor_loop_once_for(glbs.p_reactor, 1000);
		loop_counter++;
	}


	listener_dtor(&glbs.p_listener);
	reactor_dtor(&glbs.p_reactor);
	return 0;
}

typedef struct
{
	int fd;
	struct sockaddr addr;
	socklen_t addr_len;
}
my_data_t, *my_data_pt;

#include <assert.h>

static int 
my_callback_accept(const reactor_cb_args_pt inp_args)
{
	my_data_pt p_data = calloc(1, sizeof(my_data_t));
	if(p_data) {
		p_data->fd = inp_args->data.accept_args.accecpt_fd;
		p_data->addr_len = inp_args->data.accept_args.in_len;
		memcpy(&p_data->addr, inp_args->data.accept_args.p_addr, p_data->addr_len);
		// incoming ->data.accept_args.p_userdata points to &glbs set on the reactor.
		// change here to set it to the p_userdata value at the connection level.
		// Note, this doesn't change the reactor's p_userdata, it only sets the
		// per incoming fd p_userdata value which you need when processing events
		// later on in the my_callback_event() function.
		inp_args->data.accept_args.p_userdata = p_data;
		return 0;
	}
	close(inp_args->data.accept_args.accecpt_fd);
	return -1;
}

static int 
my_callback_event(const reactor_cb_args_pt inp_args)
{
	int len = 0;
	char buf[512];
	my_data_pt p_data = (my_data_pt)inp_args->data.event_args.p_userdata;
	memset(buf, 0, sizeof(buf));
	printf(" event %d %08x\n", inp_args->data.event_args.accepted_fd, inp_args->data.event_args.event);
	assert(inp_args->data.event_args.p_userdata != NULL);
	if(inp_args->data.event_args.event & REACTOR_IN) {
		while((len = read(p_data->fd, buf, sizeof(buf))) > 0) {
			printf("%s", buf);
			write(p_data->fd, buf, len); // echo back to caller.
			memset(buf, 0, sizeof(buf));
		}
		close(p_data->fd);
		free(p_data);
		return 0;
	}
	return -1;
}


static int
my_callback(const reactor_cb_args_pt inp_args)
{
	printf("Callback\n");
	switch (inp_args->type) {
	case REACTOR_ACCEPT: 
		my_callback_accept(inp_args);
		break;
	default:
		my_callback_event(inp_args);
		break;
	} 
	return 0;
}

