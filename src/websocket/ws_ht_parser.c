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

// This parser is specifically for in incoming WebSocket HTTP Upgrade Request 
// for a WebSocket. Anything else will be rejected.
//
// Makes use of the nodejs project's http_parser library:-
// https://github.com/nodejs/http-parser

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <http_parser.h>

#define MAX_BUFFER_SIZE 8192
#define MAX_HEADER_SIZE 256
#define MAX_VALUE_SIZE  512

#ifdef __cplusplus
extern "C" {
#endif

struct _userdata {
	char	header[MAX_HEADER_SIZE];
	json_t	*p_json;
	int     flag_lowercase_headers;
};
typedef struct _userdata userdata_t;

struct _header_info {
	const char *p_header;
	int required;
};
typedef struct _header_info header_info_t;

// Permit only the following headers.
static const header_info_t p_headers[] = {
	{ "uri", 			1 },
	{ "host", 			1 },
	{ "origin", 			1 },
	{ "upgrade", 			1 },
	{ "connection", 		1 },
	{ "sec-websocket-key", 		1 },
	{ "sec-websocket-accept", 	0 },
	{ "sec-websocket-version", 	0 },
	{ "sec-websocket-protocol", 	0 },
	{ "sec-websocket-extensions",	0 },
	{ NULL , 0 } // Always last.
};

static int 
on_message_begin(http_parser *inp) 
{
	return 0;
}

static int 
on_message_complete(http_parser *inp) 
{
	if(inp && inp->data) {
		userdata_t *pu = (userdata_t*)inp->data;
		json_object_set_new(pu->p_json, "method", json_string(http_method_str(inp->method)));
	}
	return 0;
}

static int 
on_headers_complete(http_parser *inp) 
{
	return 0;
}

static inline void
header_tolower(char *inp, size_t in_len) 
{
	while(in_len--) {
		*inp = tolower(*inp);
		inp++;
	}
}

static int 
on_header_field(http_parser *inp, const char *inp_value, size_t in_len) 
{
	if(inp && inp->data) {
		userdata_t *pu = (userdata_t*)inp->data;
		memset(pu->header, 0, MAX_HEADER_SIZE);
        	if(in_len > 0 && in_len < MAX_HEADER_SIZE && inp_value) {
			int i, ok = 0;
			for(i = 0; p_headers[i].p_header != NULL; i++) {
				if(strncasecmp(p_headers[i].p_header, inp_value, in_len) == 0) {
					ok = 1;
					break;
				}
			}
			if(ok) {
				memcpy(pu->header, inp_value, in_len);
				if(pu->flag_lowercase_headers) {
					header_tolower(pu->header, in_len);
				}
			} 
        	}
	}
        return 0;
}

static int 
on_header_value(http_parser *inp, const char *inp_value, size_t in_len) 
{
	if(in_len > 0 && inp_value && inp && inp->data)  {
		userdata_t *pu = (userdata_t*)inp->data;
		if(pu->header[0] != '\0') {
			json_t *p_test = json_object_get(pu->p_json, pu->header);
			if(p_test == NULL) {
				json_object_set_new(pu->p_json, pu->header, json_stringn(inp_value, in_len));
			}
			else {
				int new_len = json_string_length(p_test) + in_len + 1;
				if(new_len < MAX_VALUE_SIZE) {
					char s[MAX_VALUE_SIZE];
					memset(s, 0, MAX_VALUE_SIZE);
					memcpy(s, json_string_value(p_test), json_string_length(p_test));
					memcpy(s+json_string_length(p_test), inp_value, in_len);
					json_string_set(p_test, s);
				}
			}
		}
 	}
  	return 0;
}

static int 
on_url(http_parser *inp, const char *inp_value, size_t in_len) 
{
        if(in_len > 0 && inp_value && inp && inp->data) {
                userdata_t *pu = (userdata_t*)inp->data;
                json_object_set_new(pu->p_json, "uri", json_stringn(inp_value, in_len));
        }        
        return 0;
}

static int 
on_status(http_parser *inp, const char *inp_value, size_t in_len) 
{
	return 0;
}

static int 
on_body(http_parser *inp, const char *inp_value, size_t in_len) 
{
	return 0;
}


static 
http_parser_settings settings = {
	.on_message_begin = on_message_begin,
	.on_headers_complete = on_headers_complete,
	.on_message_complete = on_message_complete,
	.on_header_field = on_header_field,
	.on_header_value = on_header_value,
	.on_url = on_url,
	.on_status = on_status,
	.on_body = on_body,
	.on_chunk_header = NULL,
	.on_chunk_complete = NULL
};

size_t 
htparse_websocket_upgrade_request(
	const char *in_buffer, 
	int in_bufsize, 
	json_t *outp_json, 
	int *outp_is_upgrade, 
	int in_check_hdrs,
	int in_force_headers_lowercase)
{
	size_t rval;
	userdata_t userdata;
	struct http_parser parser;
	if(in_bufsize > MAX_BUFFER_SIZE || outp_json == NULL || !json_is_object(outp_json)) {
		return -1;
	}
	memset(userdata.header, 0, MAX_HEADER_SIZE);
	userdata.p_json = outp_json;
	userdata.flag_lowercase_headers = in_force_headers_lowercase;
	parser.data = &userdata;
	http_parser_init(&parser, HTTP_REQUEST);
	rval = http_parser_execute(&parser, &settings, in_buffer, in_bufsize);
	if(in_check_hdrs) {
		int i, ok = 0;
		json_t *p_value;
		const char *json_key;
		for(i = 0; p_headers[i].p_header != NULL; i++) {
			if(p_headers[i].required != 0) ok++;
		}
		json_object_foreach(outp_json, json_key, p_value) {
			for(i = 0; p_headers[i].p_header != NULL; i++) {
				if(p_headers[i].required == 0) continue;
				if(strcasecmp(json_key, p_headers[i].p_header) == 0) { 
					ok--; 
					break; 
				}
			}
		}
		if(ok != 0) {
			return -1;
		}
	}
	if(outp_is_upgrade) {
		*outp_is_upgrade = parser.upgrade ? 1 : 0;
	}
	return rval;
}

#ifdef __cplusplus
}
#endif

