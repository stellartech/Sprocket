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

// As spike project reactor.c but with SSL

// Ref https://www.ibm.com/support/knowledgecenter/en/SSB23S_1.1.0.12/gtps7/s5sple1.html

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../src/reactor.h"
#include "../src/listener.h"

#define MAXEVENTS 64

typedef struct
{
	reactor_pt 	p_reactor;
	listener_pt	p_listener;
	SSL_CTX         *p_sslctx;
}
my_globals_t;

static int
my_callback(const reactor_cb_args_pt inp_args);

int
main(int argc, char *argv[])
{
	int rc, loop_counter = 0;
	my_globals_t globals;

	SSL_load_error_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();

	globals.p_sslctx = SSL_CTX_new(TLSv1_1_method());
	SSL_CTX_set_options(globals.p_sslctx, SSL_OP_SINGLE_DH_USE);	
	rc = SSL_CTX_use_certificate_file(globals.p_sslctx, 
		"/certs/localhost_ajk_io/localhost.ajk.io.ha.pem" , SSL_FILETYPE_PEM);
	if(rc != 1) {
		printf("Error on SSL_CTX_use_certificate_file()\n");
		return 0;
	}
	rc = SSL_CTX_use_PrivateKey_file(globals.p_sslctx,
		"/certs/localhost_ajk_io/localhost.ajk.io.key", SSL_FILETYPE_PEM);
	if(rc != 1) {
		printf("Error on SSL_CTX_use_PrivateKey_file()\n");
		return 0;
	}

	globals.p_listener = listener_ctor("0.0.0.0", 443);
	if(!globals.p_listener) {
		printf("Failed ctor()\n");
		return 0;
	}

	if(listener_bind(globals.p_listener) < 1) {
		printf("Failed bind\n");
		listener_dtor(&globals.p_listener);
		return 0;
	}
	listener_set_backlog(globals.p_listener, 1024);

	{ // Create a reactor for the listening socket
		reactor_ctor_args_t args;
		args.listener_fd = listener_get_fd(globals.p_listener);
		args.event_flags = REACTOR_IN | REACTOR_OUT | REACTOR_ET | REACTOR_RDHUP;
		args.p_callback = my_callback;
		args.p_userdata = &globals;
		globals.p_reactor = reactor_ctor(&args);
	}

	// Activate the listener.
	listener_listen(globals.p_listener);

	// Basic event loop
	printf("Entering event loop\n");
	while(loop_counter < 30) {
		reactor_loop_once_for(globals.p_reactor, 1000);
		loop_counter++;
	}


	listener_dtor(&globals.p_listener);
	SSL_CTX_free(globals.p_sslctx);
	reactor_dtor(&globals.p_reactor);
	return 0;
}

typedef struct
{
	int fd;
	SSL *p_ssl;
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
		int rc;
		printf("my_callback_accept()...");
		my_globals_t *p_globals = (my_globals_t*)inp_args->data.accept_args.p_userdata;
		SSL_CTX *p_sslctx = p_globals->p_sslctx;
		if(!p_sslctx) {
			printf(" fail1\n");
			close(inp_args->data.accept_args.accecpt_fd);
			return -1;
		}
		p_data->fd = inp_args->data.accept_args.accecpt_fd;
		p_data->addr_len = inp_args->data.accept_args.in_len;
		memcpy(&p_data->addr, inp_args->data.accept_args.p_addr, p_data->addr_len);
		if((p_data->p_ssl = SSL_new(p_sslctx)) == NULL) {
			printf(" fail3\n");
			close(inp_args->data.accept_args.accecpt_fd);
			return -1;
		}
		SSL_set_fd(p_data->p_ssl, p_data->fd);
		if((rc = SSL_accept(p_data->p_ssl)) < 1) {
			rc = SSL_get_error(p_data->p_ssl, rc);
			if(rc == SSL_ERROR_WANT_READ) SSL_read(p_data->p_ssl, NULL, 0);
			else {
				printf(" fail2 %d\n", rc);
				close(inp_args->data.accept_args.accecpt_fd);
				return -1;
			}
		}
		// incoming ->data.accept_args.p_userdata points to &globals set on the reactor.
		// change here to set it to the p_userdata value at the connection level.
		// Note, this doesn't change the reactor's p_userdata, it only sets the
		// per incoming fd p_userdata value which you need when processing events
		// later on in the my_callback_event() function.
		inp_args->data.accept_args.p_userdata = p_data;
		printf(" ok\n");
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
	if(1 || inp_args->data.event_args.event & REACTOR_IN) {
		while((len = SSL_read(p_data->p_ssl, buf, sizeof(buf))) > 0) {
			printf("%s", buf);
			SSL_write(p_data->p_ssl, buf, len);
			memset(buf, 0, sizeof(buf));
		}
	 	if(len < 0) {
			int rc = SSL_get_error(p_data->p_ssl, len);
			if(rc != SSL_ERROR_WANT_READ && rc != SSL_ERROR_WANT_WRITE) {
				SSL_shutdown(p_data->p_ssl);
				SSL_free(p_data->p_ssl);
				close(p_data->fd);
				free(p_data);
			}
		}	
		if(len == 0) {
			SSL_shutdown(p_data->p_ssl);
			SSL_free(p_data->p_ssl);
			close(p_data->fd);
			free(p_data);
		}
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

