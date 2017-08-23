#ifndef SSL_CONTEXT_H_INCLUDED
#define SSL_CONTEXT_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
  
struct _ssl_context;
typedef struct _ssl_context *ssl_context_pt;

ssl_context_pt
ssl_context_ctor(void);

void
ssl_context_free(void *inp);

// Returns SSL_CTX* as a void*, cast as required.
void*
ssl_context_get(ssl_context_pt inp_self);

long
ssl_context_use_certificate_chain_file(ssl_context_pt inp_self, const char *inp_filename);

long
ssl_context_use_privatekey_file(ssl_context_pt inp_self, const char *inp_filename);

long
ssl_context_set_options(ssl_context_pt inp_self, long in_options);

long
ssl_context_clear_options(ssl_context_pt inp_self, long in_options);

long
ssl_context_get_options(ssl_context_pt inp_self);

#ifdef __cplusplus
}
#endif
  
#endif /* SSL_CONTEXT_H_INCLUDED */

