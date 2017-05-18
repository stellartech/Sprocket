


#ifndef SSL_H_INCLUDED
#define SSL_H_INCLUDED

#include "reactor.h"

#ifdef __cplusplus
extern C {
#endif

struct _ssl;
typedef struct _ssl   ssl_t;
typedef struct _ssl * ssl_pt;

typedef struct
{
	reactor_pt p_reactor;
	reactor_ctor_args_t reactor_args;
	const char *p_crt_chain_filename;
	const char *p_private_key_filename;
	void *p_userdata;
} ssl_ctor_args_t;


ssl_pt
ssl_ctor(ssl_ctor_args_t*);

void
ssl_free(void *inp);


#ifdef __cplusplus
}
#endif

#endif /* SSL_H_INCLUDED */
