

#ifndef WS_HT_PARSE_H_INCLUDED
#define WS_HT_PARSE_H_INCLUDED

#include <jansson.h>

size_t 
htparse_websocket_upgrade_request(
	const char *in_buffer, 
	int in_bufsize, 
	json_t *outp_json, 
	int *outp_is_upgrade, 
	int in_check_hdrs);

#endif /* WS_HT_PARSE_H_INCLUDED */
