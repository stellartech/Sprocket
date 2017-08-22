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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "../src/utils/base64.h"
#include "../src/utils/ws_magic.h"

static uint8_t accept_string[] = "dGhlIHNhbXBsZSBub25jZQ==";
static uint8_t response_string[] = "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=";
static uint8_t sha1_string[] = {
	0xb3, 0x7a, 0x4f, 0x2c, 0xc0, 0x62, 0x4f, 0x16, 
	0x90, 0xf6, 0x46, 0x06, 0xcf, 0x38, 0x59, 0x45, 
	0xb2, 0xbe, 0xc4, 0xea
}; 

void setup(void)
{
}

void teardown(void)
{
}

START_TEST(test_ws_magic__make_websocket_accept_response)
{
	int i;
	char *p_base64;
	const unsigned char *ws_accept_response;
	ws_accept_response = make_websocket_accept_response(accept_string);
	ck_assert_str_eq(ws_accept_response, response_string);
	free((void*)ws_accept_response);
}
END_TEST

static const char
expect_magic_http_response1[] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: foo\r\n\r\n";

START_TEST(test_ws_magic__make_websocket_magic_response_1)
{
	char *p_response = make_websocket_magic_response("foo", NULL, NULL);
	ck_assert_str_eq(p_response, expect_magic_http_response1);
	free(p_response);
}
END_TEST

static const char
expect_magic_http_response2[] =
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: foo\r\n"
	"Sec-WebSocket-Protocol: bar\r\n\r\n";

START_TEST(test_ws_magic__make_websocket_magic_response_2)
{
	char *p_response = make_websocket_magic_response("foo", "bar", NULL);
	ck_assert_str_eq(p_response, expect_magic_http_response2);
	free(p_response);
}
END_TEST


Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("WS_MAGIC");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_ws_magic__make_websocket_accept_response);
	tcase_add_test(tc_core, test_ws_magic__make_websocket_magic_response_1);
	tcase_add_test(tc_core, test_ws_magic__make_websocket_magic_response_2);
	suite_add_tcase(s, tc_core);

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
	srunner_set_log (sr, "ws_magic.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

