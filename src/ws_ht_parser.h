

#ifndef WS_HT_PARSE_H_INCLUDED
#define WS_HT_PARSE_H_INCLUDED

#include <jansson.h>

/**
 *
 * @param in_buffer The received data buffer
 * @param in_bufsize The length of the above buffer
 * @param outp_json A json_t object to received the parsed headers
 * @param outp_is_upgrade A flag, 0 if no Upgrade header received, non-zero otherwise
 * @param in_check_hdrs Check the valid WebSocket Upgrade headers are all present
 * @param in_force_headers_lowerase Force headers to lower case, makes searching for them easier later.
 * @return Number of bytes processed
 */ 
size_t 
htparse_websocket_upgrade_request(
	const char *in_buffer,		
	int in_bufsize, 
	json_t *outp_json, 
	int *outp_is_upgrade, 
	int in_check_hdrs,
	int in_force_headers_lowerase
);

#endif /* WS_HT_PARSE_H_INCLUDED */
