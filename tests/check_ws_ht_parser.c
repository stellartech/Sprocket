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

#include <check.h>

#include "websocket/ws_ht_parser.h"

static const char data[] =
	"GET /connect/0123456789012345678901234567891 HTTP/1.1\r\n"
	"Host: example.com\r\n"
	"Upgrade: WebSocket\r\n"
	"Connection: Upgrade\r\n"
	"Origin: http://example.com\r\n"
	"Sec-WebSocket-Key: foobarbaz\r\n"
	"Sec-WebSocket-Extensions: websocket-extensions\r\n"
	"Sec-WebSocket-Accept: websocket-accepts\r\n" 
	"Sec-WebSocket-Protocol: chat, superchat\r\n"
	"Sec-WebSocket-Version: 13\r\n"
	"WebSocket-Protocol: sample\r\n\r\n";
static const size_t data_len = sizeof(data) - 1;

static inline void
string_test(json_t *inp, const char *key, const char *val)
{
	ck_assert(json_is_object(inp));
	ck_assert(json_is_string(json_object_get(inp, key)));
	ck_assert_str_eq(val, json_string_value(json_object_get(inp, key)));
}

json_t *p = NULL;

void setup(void)
{
	int is_upgrade = 0;
	size_t rval = 0;
	char *p_json_dump = NULL;
	const char *p_string;
	json_t *p_test = NULL;
	json_error_t j_err;

	p = json_object();
	rval = htparse_websocket_upgrade_request(data, data_len, p, &is_upgrade, 0, 1);
	p_json_dump = json_dumps(p, 0);
	ck_assert(p_json_dump);
	json_decref(p); // Destroy original object.

	p = json_loads(p_json_dump, 0, &j_err); // And reload it via json string.
	free(p_json_dump); // Now destroy the temp string.
}

void teardown(void)
{
	if(p) {
		json_decref(p);
		p = NULL;
	}
}

START_TEST(ws_ht_parser)
{
	string_test(p, "uri", "/connect/0123456789012345678901234567891");
	string_test(p, "host", "example.com");
	string_test(p, "method", "GET");
	string_test(p, "upgrade", "WebSocket");
	string_test(p, "connection", "Upgrade");
	string_test(p, "sec-websocket-key", "foobarbaz");
	string_test(p, "sec-websocket-extensions", "websocket-extensions");
}
END_TEST

START_TEST(ws_ht_parser_filters)
{
	ck_assert(json_object_get(p, "WebSocket-Protocol") == NULL);
	ck_assert(json_object_get(p, "websocket-protocol") == NULL);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;
	TCase *tc_extra;

	s = suite_create("WS_HT_PARSER");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, ws_ht_parser);
	suite_add_tcase(s, tc_core);

	tc_extra = tcase_create("Extra");
	tcase_add_checked_fixture(tc_extra, setup, teardown);
	tcase_add_test(tc_extra, ws_ht_parser_filters);
	suite_add_tcase(s, tc_extra);
	return s;
}

int 
main(void)
{
	int number_failed;
	Suite *s;
	SRunner *sr;
	s = suite();
	sr = srunner_create(s);
	srunner_set_log (sr, "ws_ht_parser.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

