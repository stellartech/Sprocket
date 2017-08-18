

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>


#include "ssl.h"
#include "conn_if.h"
#include "utils/iovarr.h"

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
	conn_t	conn; // Must be defined first in struct.
        int fd;
        int state;
        SSL *p_ssl;
	iovarr_pt p_out_iovarr;
	uint64_t bytes_in;
	uint64_t bytes_out;
	void *p_userdata;
}
my_data_t, *my_data_pt;

///

static void 
ssl_reactor_close_sock(my_data_pt inp_data)
{

	SSL_shutdown(inp_data->p_ssl);
	SSL_free(inp_data->p_ssl);
	ERR_remove_thread_state(NULL);
	close(inp_data->fd);
	free(inp_data);
}

// conn_if
static void
ssl_reactor_conn_if_close(conn_pt inp_conn)
{
	my_data_pt p_self = (my_data_pt)inp_conn;
	ssl_reactor_close_sock(p_self);
}

static iovarr_pt
ssl_reactor_conn_if_read(conn_pt inp_conn)
{
	my_data_pt p_self = (my_data_pt)inp_conn;
	return NULL;
}

static int
ssl_reactor_conn_if_write(conn_pt inp_conn, void *inp_buf, int in_buflen)
{
	my_data_pt p_self = (my_data_pt)inp_conn;
	if(!p_self->p_out_iovarr) return -1;
	if(iovarr_count(p_self->p_out_iovarr) != 0) {
		// If there is a waiting queue to write
		// then append this buffer to that queue
		// and tell the caller they were sent.
		struct iovec vec;
		vec.iov_base = inp_buf;
		vec.iov_len = in_buflen;
		iovarr_pushback(p_self->p_out_iovarr, &vec);
		return in_buflen;
	}
	else {
		// If the output buffer is empty try sending immediately
		// and handle SSL as required. 
		int len = SSL_write(p_self->p_ssl, inp_buf, in_buflen);
		if(len == in_buflen) {
			free(inp_buf);
			return in_buflen;
		}
		else if(len < 1) {
			switch(SSL_get_error(p_self->p_ssl, len)) {
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_NONE:
			{
				struct iovec vec;
				vec.iov_base = inp_buf;
				vec.iov_len = in_buflen;
				iovarr_pushback(p_self->p_out_iovarr, &vec);
				return in_buflen;
			}
			default:
				// Callback to upper layer, closing!
				ssl_reactor_close_sock(p_self);
				free(inp_buf);
				return -1;
			}
		}
		else {
			// If we sent some but not all bytes in the 
			// buffer then requeue these remain bytes
			// and tell teh caller they all went.
			struct iovec vec;
			int remaining = in_buflen - len;
			void *dst = inp_buf;
			void *src = inp_buf + len;
			memcpy(dst, src, remaining);
			vec.iov_base = inp_buf;
			vec.iov_len = remaining;
			iovarr_pushback(p_self->p_out_iovarr, &vec);
			return in_buflen;
		}
	}
	return -1; // Should never reach here.
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
	p_data->p_userdata = p_self;
	p_data->fd = inp_args->data.accept_args.accecpt_fd;
	p_data->conn.iface.close = ssl_reactor_conn_if_close;
	p_data->conn.iface.read = ssl_reactor_conn_if_read;
	p_data->conn.iface.write = ssl_reactor_conn_if_write;
	p_data->conn.addrlen = inp_args->data.accept_args.in_len;
	memcpy(&p_data->conn.addr, inp_args->data.accept_args.p_addr, p_data->conn.addrlen);
	if((p_data->p_ssl = SSL_new(p_self->p_sslctx)) == NULL) {
		close(inp_args->data.accept_args.accecpt_fd);
		free(p_data);
		return -1;
	}	
	SSL_set_fd(p_data->p_ssl, p_data->fd);
	if((rc = SSL_accept(p_data->p_ssl)) < 1) {
		rc = SSL_get_error(p_data->p_ssl, rc);
		if(rc != SSL_ERROR_WANT_READ && rc != SSL_ERROR_WANT_WRITE) {
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
	int len, pending_len;
	iovarr_pt p_iovarr;
	struct iovec iovec;

	if((p_iovarr = iovarr_ctor()) == NULL) {
		ssl_reactor_close_sock(inp_data);
		return -1;
	}
	if((pending_len = SSL_pending(inp_data->p_ssl)) == 0) {
		pending_len = 4096;
	}
	while(pending_len > 0) {
		if((pbuf = calloc(1, pending_len)) == NULL) {
			ssl_reactor_close_sock(inp_data);
			iovarr_decref(p_iovarr);
			return -1;
		}
		len = SSL_read(inp_data->p_ssl, pbuf, pending_len);
		if(len > 0) {
			iovec.iov_base = pbuf; 
			iovec.iov_len = len;
			iovarr_pushback(p_iovarr, &iovec);
			inp_data->bytes_in += len;
			pending_len = SSL_pending(inp_data->p_ssl);
		}	
		else if(len < 0) {
			int rc = SSL_get_error(inp_data->p_ssl, len);
			switch(rc) {
			case SSL_ERROR_WANT_READ:
			case SSL_ERROR_WANT_WRITE:
			case SSL_ERROR_NONE:
				// Reactor will call again in the future
				// to meet the needs of SSL_read or SSL_write.
				free(pbuf);
				// Fallthru.
			default:
				pending_len = SSL_pending(inp_data->p_ssl);
			}
		}
		else {
			free(pbuf);
			ssl_reactor_close_sock(inp_data);
			iovarr_decref(p_iovarr);
			return 0;
		}
	}	

	if(iovarr_count(p_iovarr) > 0) {
		// Make callback to upper layer
		// passing this conn data and the
		// iovarr of payload data.
		// The callback can steal the
		// iovarr if it wants to.

		// If it didn't steal it, then
		// free it.
		iovarr_decref(p_iovarr);
	}

	return 1;
}

static int
ssl_reactor_write(my_data_pt inp_data)
{
	if(!inp_data) return -1;
	if(!inp_data->p_out_iovarr) return 0;
	if(iovarr_count(inp_data->p_out_iovarr) < 1) return 0;
	else {
		struct iovec *p_iovec = iovarr_ref(inp_data->p_out_iovarr);
		while(p_iovec && iovarr_count(inp_data->p_out_iovarr) > 0) {
			int len = SSL_write(inp_data->p_ssl, p_iovec[0].iov_base, p_iovec[0].iov_len);
			if(len == p_iovec[0].iov_len) {
				// Take ownership of the sent iov_base buffer and free it.
				void *pbuf = iovarr_popfront(inp_data->p_out_iovarr, NULL);
				if(pbuf) free(pbuf);
			}
			else if(len < 1) {
			}
			else { 
				// Partial buffer send, requeue remaining bytes.
				// in the iov buffer.
				int remaining = p_iovec[0].iov_len - len;
				void *dst = p_iovec[0].iov_base;
				void *src = dst + len;
				memcpy(dst, src, remaining);
				p_iovec[0].iov_len = remaining;
			}
		}
		return 0;
	}
	return -1;
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

