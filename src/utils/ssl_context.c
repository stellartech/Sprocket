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

#include <stdio.h>
#include <stdlib.h>

#include "ssl_context.h"

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#ifdef __cplusplus
extern "C" {
#endif
  
struct _ssl_context
{
	int refcount;
	SSL_CTX	*p_context;
};
typedef struct _ssl_context ssl_context_t;

ssl_context_pt
ssl_context_ctor(void)
{
	ssl_context_pt p_self = calloc(1, sizeof(ssl_context_t));
	if(p_self) {
		p_self->refcount = 1;
		SSL_load_error_strings();
		SSL_library_init();
		OpenSSL_add_all_algorithms();
		p_self->p_context = SSL_CTX_new(TLSv1_2_method());
		ssl_context_set_options(p_self, SSL_OP_SINGLE_DH_USE);
	}
	return p_self;
}

static void
_ssl_context_clean_up_library(ssl_context_pt inp_self)
{
	// The following ensures SSL lib doesn't
	// leak mem under valgrind tests, sigh
	ERR_remove_thread_state(NULL);
	ENGINE_cleanup();
	CONF_modules_free();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
	SSL_COMP_free_compression_methods();
	ERR_free_strings();
}

void
ssl_context_free(void *inp)
{
	if(inp) {
		ssl_context_pt p_self = (ssl_context_pt)inp;
		__sync_fetch_and_sub(&p_self->refcount, 1);
		if(p_self->refcount < 1) {
			SSL_CTX_free(p_self->p_context);
			_ssl_context_clean_up_library(p_self);
			free(p_self);
		}
	}
}

void*
ssl_context_get(ssl_context_pt inp_self)
{
	return (void*)inp_self->p_context;
}

long
ssl_context_use_certificate_chain_file(ssl_context_pt inp_self, 
	const char *inp_filename)
{
	return SSL_CTX_use_certificate_chain_file(
		inp_self->p_context, inp_filename);
}

long
ssl_context_use_privatekey_file(ssl_context_pt inp_self, 
	const char *inp_filename)
{
	return SSL_CTX_use_PrivateKey_file(inp_self->p_context, 
		inp_filename, SSL_FILETYPE_PEM);
}

long
ssl_context_set_options(ssl_context_pt inp_self, long in_options)
{
	return SSL_CTX_set_options(inp_self->p_context, in_options);
}

long
ssl_context_clear_options(ssl_context_pt inp_self, long in_options)
{
	return SSL_CTX_clear_options(inp_self->p_context, in_options);
}

long
ssl_context_get_options(ssl_context_pt inp_self)
{
	return SSL_CTX_get_options(inp_self->p_context);
}

#ifdef __cplusplus
}
#endif
  

