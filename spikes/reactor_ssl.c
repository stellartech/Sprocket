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
//
// SSL Gold
// http://stackoverflow.com/questions/29845527/how-to-properly-uninitialize-openssl

#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#include <netdb.h>

#include "../src/reactor.h"
#include "../src/listener.h"

#define MAXEVENTS 64

typedef struct
{
	int ev;
	const char *p_name;
} n_events_t;

static n_events_t n_events[] = 
{
	  { 0x0001, "EPOLLIN" }
	, { 0x0002, "EPOLLPRI" }
	, { 0x0004, "EPOLLOUT" }
	, { 0x0008, "EPOLLERR" }
	, { 0x0010, "EPOLLHUP" }
	, { 0x0040, "EPOLLRDNORM" }
	, { 0x0080, "EPOLLRDBAND" }
	, { 0x0100, "EPOLLWRNORM" }
	, { 0x0200, "EPOLLWRBAND" }
	, { 0x0400, "EPOLLMSG" }
	, { 0x2000, "EPOLLRDHUP" }
	, { (1<<29), "EPOLLWAKEUP" }
	, { (1<<30), "EPOLLONESHOT" }
	, { (1<<31), "EPOLLET" }
	, { 0, "" }
};


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

	globals.p_sslctx = SSL_CTX_new(TLSv1_2_method());
	SSL_CTX_set_options(globals.p_sslctx, SSL_OP_SINGLE_DH_USE);	

	rc = SSL_CTX_use_certificate_chain_file(globals.p_sslctx, 
		"/certs/localhost_ajk_io/localhost.ajk.io.ha.pem");
	if(rc != 1) {
		fprintf(stderr, "Error on SSL_CTX_use_certificate_file()\n");
		return 0;
	}
	rc = SSL_CTX_use_PrivateKey_file(globals.p_sslctx,
		"/certs/localhost_ajk_io/localhost.ajk.io.key", SSL_FILETYPE_PEM);
	if(rc != 1) {
		fprintf(stderr, "Error on SSL_CTX_use_PrivateKey_file()\n");
		return 0;
	}

	globals.p_listener = listener_ctor("0.0.0.0", 443);
	if(!globals.p_listener) {
		fprintf(stderr, "Failed ctor()\n");
		return 0;
	}

	if(listener_bind(globals.p_listener) < 1) {
		fprintf(stderr, "Failed bind\n");
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
	fprintf(stderr, "Entering event loop\n");
	while(loop_counter < 10) {
		reactor_loop_once_for(globals.p_reactor, 1000);
		loop_counter++;
	}


	listener_dtor(&globals.p_listener);

	// Wow, OpenSSL requires some cleanup on exit!
	// All these were needed to make Valgrind happy.
	SSL_CTX_free(globals.p_sslctx);
	ERR_remove_thread_state(NULL);
	ENGINE_cleanup();
	CONF_modules_free();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	SSL_COMP_free_compression_methods();
	ERR_free_strings();

	reactor_dtor(&globals.p_reactor);
	return 0;
}

typedef struct
{
	int fd;
	int state;
	SSL *p_ssl;
	struct sockaddr addr;
	socklen_t addr_len;
	char remote_ip[NI_NUMERICHOST];
	char remote_port[NI_NUMERICSERV];
}
my_data_t, *my_data_pt;

#include <assert.h>

static int 
my_callback_accept(const reactor_cb_args_pt inp_args)
{
	my_data_pt p_data = calloc(1, sizeof(my_data_t));
	if(p_data) {
		int rc;
		fprintf(stderr, "my_callback_accept()...");
		my_globals_t *p_globals = (my_globals_t*)inp_args->data.accept_args.p_userdata;
		SSL_CTX *p_sslctx = p_globals->p_sslctx;
		if(!p_sslctx) {
			fprintf(stderr, " fail1\n");
			close(inp_args->data.accept_args.accecpt_fd);
			return -1;
		}
		p_data->fd = inp_args->data.accept_args.accecpt_fd;
		p_data->addr_len = inp_args->data.accept_args.in_len;
		memcpy(&p_data->addr, inp_args->data.accept_args.p_addr, p_data->addr_len);
		if((p_data->p_ssl = SSL_new(p_sslctx)) == NULL) {
			fprintf(stderr, " fail3\n");
			close(inp_args->data.accept_args.accecpt_fd);
			return -1;
		}
		SSL_set_fd(p_data->p_ssl, p_data->fd);
		if((rc = SSL_accept(p_data->p_ssl)) < 1) {
			rc = SSL_get_error(p_data->p_ssl, rc);
			if(rc == SSL_ERROR_WANT_READ) SSL_read(p_data->p_ssl, NULL, 0);
			else {
				fprintf(stderr, " fail2 %d\n", rc);
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

		{ // Log incoming
			char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
			rc = getnameinfo(&p_data->addr, p_data->addr_len,
				hbuf, sizeof(hbuf), sbuf, sizeof(sbuf),
				NI_NUMERICHOST | NI_NUMERICSERV);
			if(rc == 0) {
				fprintf(stderr, "Accepted conn on %d '%s:%s'\n",
					p_data->fd, hbuf, sbuf);
			}
			else {
				fprintf(stderr, "Failed with %d\n", rc);
			}

		}
		return 0;
	}
	close(inp_args->data.accept_args.accecpt_fd);
	return -1;
}

int once = -1;
const char response[] = 
	"HTTP/1.1 200 OK\r\n"
	"Content-Length: 7\r\n\r\n"
	"hello\r\n";

static int
my_callback_write(my_data_pt inp_data)
{
	// ToDo
	int rc = SSL_get_state(inp_data->p_ssl);
	fprintf(stderr, "my_callback_write() state %d\n", rc);
	rc = SSL_write(inp_data->p_ssl, NULL, 0);
	if(rc < 0) {
		rc = SSL_get_error(inp_data->p_ssl, rc);
		switch(rc) {
			case SSL_ERROR_WANT_READ:
				fprintf(stderr, "\tSSL_ERROR_WANT_READ\n");
				SSL_read(inp_data->p_ssl, NULL, 0);
				break;
			case SSL_ERROR_WANT_WRITE:
				fprintf(stderr, "\tSSL_ERROR_WANT_WRITE\n");
				SSL_write(inp_data->p_ssl, NULL, 0);
				break;		
			default:
				SSL_shutdown(inp_data->p_ssl);
				SSL_free(inp_data->p_ssl);
				ERR_remove_thread_state(NULL);
				close(inp_data->fd);
				free(inp_data);
				break;	
		}
	}	
	if(once == 0) {
		fprintf(stderr, "\tSending dummy response\n");
		SSL_write(inp_data->p_ssl, response, sizeof(response)-1);
		once++;
	}
	// ToDo, we need a go over the output buffer list and
	// write out any pending data.
	return 0;
}

static int
my_callback_read(my_data_pt inp_data)
{
	int rc, len = 0, buf_len = 0;
	char *s, *pbuf;

	if((buf_len = SSL_pending(inp_data->p_ssl)) == 0) {
		buf_len = 4096;
	}

	pbuf = malloc(buf_len);

	fprintf(stderr, "my_callback_read() pending: %d\n", SSL_pending(inp_data->p_ssl));
	while(1) { //(len = SSL_read(inp_data->p_ssl, buf, sizeof(buf))) > 0) {
		memset(pbuf, 0, buf_len);
		len = SSL_read(inp_data->p_ssl, pbuf, buf_len);
		s = strndup(pbuf, len);
		fprintf(stderr, "\tmy_callback_read() len = %d pending = %d\n\t%s\n", len, SSL_pending(inp_data->p_ssl), s);
		free(s);
		if(len > 0) {
			once = 0;
			if((buf_len = SSL_pending(inp_data->p_ssl)) == 0) {
				buf_len = 4096;
			}
			inp_data->state = 1;
			free(pbuf);
			pbuf = malloc(buf_len);
		}
		if(len < 1) break;
	}
	if(pbuf) free(pbuf);
	if(len < 0) {
		int rc = SSL_get_error(inp_data->p_ssl, len);
		switch(rc) {
			case SSL_ERROR_WANT_READ:
				//len = SSL_read(inp_data->p_ssl, NULL, 0);
				fprintf(stderr, "\tSSL_ERROR_WANT_READ %d\n", len);
				break;
			case SSL_ERROR_WANT_WRITE:
				//len = SSL_write(inp_data->p_ssl, NULL, 0);
				fprintf(stderr, "\tSSL_ERROR_WANT_WRITE %d\n", len);
				break;		
			default:
				fprintf(stderr, "\tSSL_ERROR_UNKNOWN %d\n", rc);
				SSL_shutdown(inp_data->p_ssl);
				SSL_free(inp_data->p_ssl);
				ERR_remove_thread_state(NULL);
				close(inp_data->fd);
				free(inp_data);
				break;	
		}
	}
	return len;
}

static int 
my_callback_event(const reactor_cb_args_pt inp_args)
{
	int sh = 1;
	int fd = inp_args->data.event_args.accepted_fd;
	int ev = inp_args->data.event_args.event;
	my_data_pt p_data = (my_data_pt)inp_args->data.event_args.p_userdata;

	fprintf(stderr, "my_callback_event() fd %d ev %08x\n", fd, ev);
	assert(inp_args->data.event_args.p_userdata != NULL);
	if(ev & REACTOR_RDHUP) {
		fprintf(stderr, "\tConnection reset by peer\n");
		SSL_shutdown(p_data->p_ssl);
		SSL_free(p_data->p_ssl);
		ERR_remove_thread_state(NULL);
		close(p_data->fd);
		free(p_data);
		return 0;
	}
	if(ev & REACTOR_IN) {
		sh = my_callback_read(p_data);
	}
	if(0 && ev & REACTOR_OUT) {
		my_callback_write(p_data);		
	}
	if(sh == 0) {
		fprintf(stderr, "\tConnection reset by peer\n");
		SSL_shutdown(p_data->p_ssl);
		SSL_free(p_data->p_ssl);
		ERR_remove_thread_state(NULL);
		close(p_data->fd);
		free(p_data);
		return 0;
	}
	if(ev & REACTOR_OUT) {
		my_callback_write(p_data);		
	}
	return 0;
}

static const char*
get_event_name(int in_ev, int from, int *at)
{
	for(int i = from; n_events[i].ev > 0; i++) {
		if(in_ev & n_events[i].ev) {
			if(at) *at = i;
			return n_events[i].p_name;
		}
	}
	return NULL;
}

static int
my_callback(const reactor_cb_args_pt inp_args)
{
	int i = 0;
	int ev = inp_args->data.event_args.event;
	const char *p_name = NULL;
	fprintf(stderr, "my_callback() %08x\n", ev);
	
	p_name = get_event_name(ev, i, &i);
	while(p_name) {
		fprintf(stderr, "\tev: %s\n", p_name);
		p_name = get_event_name(ev, i+1, &i);
	}
		
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

