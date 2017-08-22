
#ifndef WS_MAGIC_H_INCLUDED
#define WS_MAGIC_H_INCLUDED
  
#include <stdint.h>
  
#ifdef  __cplusplus
extern "C" {
#endif
  
/**
 * @param outp_sha1buf Must be SHA_DIGEST_LENGTH in len.
 */
//unsigned char*
//make_websocket_accept_response_sha1(const char* inp_keystr, unsigned char *outp_sha1buf);

const char*
make_websocket_accept_response(const char* inp_keystr);
  
char*
make_websocket_magic_response(const char *inp_accept,
        const char *inp_proto, int *out_len);

  
#ifdef  __cplusplus
}
#endif
  
#endif  /* WS_MAGIC_H_INCLUDED */

