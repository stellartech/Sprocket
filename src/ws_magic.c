
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <openssl/sha.h>
  
#include "base64.h"
#include "ws_magic.h"
  
#ifdef  __cplusplus
extern "C" {
#endif

static const char* magic_key = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
  
// https://tools.ietf.org/html/rfc6455
// Opening Handshake tools and utils.

/**
 * @param outp_sha1buf Must be SHA_DIGEST_LENGTH in len.
 */
unsigned char*
make_websocket_accept_response_sha1(const char* inp_keystr, unsigned char *outp_sha1buf)
{
	unsigned char sha1buf[SHA_DIGEST_LENGTH];
	const unsigned char* p_result = NULL;
	const unsigned char* p_buf = NULL;
	int len, i = snprintf(NULL, 0, "%s%s", inp_keystr, (char*)magic_key) + 1;
	memset((char*)sha1buf, 0, SHA_DIGEST_LENGTH);
	p_buf = calloc(1, i+1);
	if(p_buf) {
		SHA_CTX ctx;
		SHA1_Init(&ctx);
		i = snprintf((char*)p_buf, i, "%s%s", inp_keystr, (char*)magic_key);
		SHA1_Update(&ctx, p_buf, i);
		SHA1_Final(sha1buf, &ctx);
		free((void*)p_buf);
		memcpy(outp_sha1buf, sha1buf, SHA_DIGEST_LENGTH);
		return outp_sha1buf;
	}
	return NULL;
}
  
const char*
make_websocket_accept_response(const char* inp_keystr)
{
	unsigned char sha1buf[SHA_DIGEST_LENGTH];
	const unsigned char* p_result;
	make_websocket_accept_response_sha1(inp_keystr, sha1buf);
	p_result = base64_encode(sha1buf, SHA_DIGEST_LENGTH, NULL);
	return (const char*)p_result;
}
  
static const char
magic_http_response[] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "%s%s%s";
  
char*
make_websocket_magic_response(const char *inp_accept,
        const char *inp_proto, int *out_len)
{
	char *p;
	int len;
  
	if(inp_proto) {
		len = snprintf(NULL, 0, magic_http_response, inp_accept,
			      "Sec-WebSocket-Protocol: ", inp_proto, "\r\n\r\n") + 1;
	}
	else {
		len = snprintf(NULL, 0, magic_http_response, inp_accept,
			      "\r\n", "", "") + 1;
	}
  
	p = calloc(1, len);
	if(p) {
		if(inp_proto) {
			len = snprintf(p, len, magic_http_response, inp_accept,
				      "Sec-WebSocket-Protocol: ", inp_proto, "\r\n\r\n");
		}
		else {
			len = snprintf(p, len, magic_http_response, inp_accept,
				      "\r\n", "", "");
		}
	}
  
	if(out_len) *out_len = len;
  
	return p;
}

#ifdef  __cplusplus
}
#endif

