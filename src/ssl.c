

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#include "ssl.h"

#ifdef __cplusplus
extern C {
#endif

struct _ssl
{
	int refcount;
	reactor_pt p_reactor;
	SSL_CTX	*p_sslctx;
	void *p_userdata;
};

static int 
ssl_reactor_cb(const reactor_cb_args_pt inp_args);

ssl_pt
ssl_ctor(ssl_ctor_args_t *inp_args)
{
	ssl_pt p_self = calloc(1, sizeof(ssl_t));
	if(p_self) {
		reactor_ctor_args_t reactor_args;
		p_self->refcount = __sync_fetch_and_add(&p_self->refcount, 1);
		if((p_self->p_sslctx = SSL_CTX_new(TLSv1_2_method())) == NULL)
			goto ssl_ctor_fail;
		if(SSL_CTX_use_certificate_chain_file(p_self->p_sslctx, 
			inp_args->p_crt_chain_filename) == -1)
				goto ssl_ctor_fail;
		if(SSL_CTX_use_PrivateKey_file(p_self->p_sslctx, 
			inp_args->p_private_key_filename, SSL_FILETYPE_PEM) == -1)
				goto ssl_ctor_fail;	
		p_self->p_userdata = inp_args->p_userdata;
		p_self->p_reactor = inp_args->p_reactor;
		reactor_set_userdata(p_self->p_reactor, p_self);
		reactor_set_cb(p_self->p_reactor, ssl_reactor_cb);
	}
	return p_self;
	ssl_ctor_fail:
	ssl_free(p_self);
	return NULL;
}

void
ssl_free(void *inp)
{
	if(inp) {
		ssl_pt p_self = (ssl_pt)inp;
		p_self->refcount = __sync_fetch_and_sub(&p_self->refcount, 1);
		if(p_self->refcount == 0) {
			SSL_CTX_free(p_self->p_sslctx);
			free(inp);
		}
	}
}

///
typedef struct
{
        int fd;
        int state;
        SSL *p_ssl;
        struct sockaddr addr;
        socklen_t addr_len;
        //char remote_ip[NI_NUMERICHOST];
        //char remote_port[NI_NUMERICSERV];
}
my_data_t, *my_data_pt;

static void 
ssl_reactor_close_sock(my_data_pt inp_data)
{

	SSL_shutdown(inp_data->p_ssl);
	SSL_free(inp_data->p_ssl);
	ERR_remove_thread_state(NULL);
	close(inp_data->fd);
	free(inp_data);
}

static int
ssl_reactor_cb_accept(const reactor_cb_args_pt inp_args)
{
	int rc;
	ssl_pt p_self;
	my_data_pt p_data = calloc(1, sizeof(my_data_t));
	if(!p_data) {
		close(inp_args->data.accept_args.accecpt_fd);
		return -1;
	}
	p_self = (ssl_pt)inp_args->data.accept_args.p_userdata;
	p_data->fd = inp_args->data.accept_args.accecpt_fd;
	p_data->addr_len = inp_args->data.accept_args.in_len;
	memcpy(&p_data->addr, inp_args->data.accept_args.p_addr, p_data->addr_len);
	if((p_data->p_ssl = SSL_new(p_self->p_sslctx)) == NULL) {
		close(inp_args->data.accept_args.accecpt_fd);
		free(p_data);
		return -1;
	}	
	SSL_set_fd(p_data->p_ssl, p_data->fd);
	if((rc = SSL_accept(p_data->p_ssl)) < 1) {
		rc = SSL_get_error(p_data->p_ssl, rc);
		if(rc == SSL_ERROR_WANT_READ) SSL_read(p_data->p_ssl, NULL, 0);
		else {
			close(inp_args->data.accept_args.accecpt_fd);
			free(p_data);
			return -1;
		}
	}
	inp_args->data.accept_args.p_userdata = p_data;
	return 0;
}

static int
ssl_reactor_read(my_data_pt inp_data)
{
	void *pbuf;
	int pending_len, len;
	struct iovec *p_iovec;

	p_iovec = calloc(1, sizeof(struct iovec));
	if(!p_iovec) {
		ssl_reactor_close_sock(inp_data);
		return -1;
	}
	if((pending_len = SSL_pending(inp_data->p_ssl)) == 0) {
		pending_len = 4096;
	}
	if((pbuf = calloc(1, pending_len)) == NULL) {
		free(p_iovec);
		ssl_reactor_close_sock(inp_data);
		return -1;
	}
	for(;;) {
		len = SSL_read(inp_data->p_ssl, pbuf, pending_len);
		if(len > 0) {
			p_iovec->iov_base = pbuf;
			p_iovec->iov_len = len;
			pending_len = SSL_pending(inp_data->p_ssl);
			if(pending_len) {
				// New iovec struct and buffer
			}	
		}
	}	

	return 1;
}

static int
ssl_reactor_cb_event(const reactor_cb_args_pt inp_args)
{
	int shut = 1;
	int ev = inp_args->data.event_args.event;
	int fd = inp_args->data.event_args.accepted_fd;
	my_data_pt p_data = (my_data_pt)inp_args->data.event_args.p_userdata;
	if(ev & REACTOR_RDHUP) {
		SSL_shutdown(p_data->p_ssl);
		SSL_free(p_data->p_ssl);
		ERR_remove_thread_state(NULL);
		close(p_data->fd);
		free(p_data);
		return 0;
	}
	if(ev & REACTOR_IN) {
		shut = ssl_reactor_read(p_data);
	}
	if(0 && ev & REACTOR_OUT) {
	}
	if(shut == 0) {
		SSL_shutdown(p_data->p_ssl);
		SSL_free(p_data->p_ssl);
		ERR_remove_thread_state(NULL);
		close(p_data->fd);
		free(p_data);
	}
	return 0;
}

static int
ssl_reactor_cb(const reactor_cb_args_pt inp_args)
{
	switch (inp_args->type) {
	case REACTOR_ACCEPT:
		return ssl_reactor_cb_accept(inp_args);
	default:
		return ssl_reactor_cb_event(inp_args);
	}
	return 0;
}
#ifdef __cplusplus
}
#endif

