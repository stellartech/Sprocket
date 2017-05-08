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
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#define WS_FRAME_FRIEND
#include "../src/ws_frame.h"

ws_frame_pt p_frame = NULL;

void setup(void)
{
	p_frame = ws_frame_ctor();
}

void teardown(void)
{
	ws_frame_dtor(&p_frame);
}

START_TEST(test_ws_frame_ctor)
{
	ck_assert(p_frame);
}
END_TEST

#define RANDOM_BUFFER_SIZE (65535 + 65535)
char random_buffer[RANDOM_BUFFER_SIZE];

static ssize_t 
load_random_buffer(ssize_t in_how_much)
{
	ssize_t randomDataRead = 0;
	int randomFd = open("/dev/urandom", O_RDONLY);
	if(randomFd) {
		randomDataRead = read(randomFd, random_buffer, in_how_much);
		close(randomFd);	
	}
	return randomDataRead;
}


START_TEST(test_ws_frame_buf_124)
{
	ssize_t how_many = load_random_buffer(256);
	unsigned char test_buf_a[256];
	test_buf_a[0] = 0;
	test_buf_a[1] = 124;
	memcpy(&test_buf_a[2], random_buffer, 124);
	ws_frame_append_chunk(p_frame, test_buf_a, 126);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert_int_eq(p_frame->frame_in, 126);
	ck_assert_int_eq(p_frame->payload_len, 124);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 124) == 0);

}
END_TEST

START_TEST(test_ws_frame_buf_125)
{
	ssize_t how_many = load_random_buffer(256);
	unsigned char test_buf_a[256];
	test_buf_a[0] = 0;
	test_buf_a[1] = 125;
	memcpy(&test_buf_a[2], random_buffer, 125);
	ws_frame_append_chunk(p_frame, test_buf_a, 127);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert_int_eq(p_frame->frame_in, 127);
	ck_assert_int_eq(p_frame->payload_len, 125);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 125) == 0);
}
END_TEST

START_TEST(test_ws_frame_buf_126)
{
	ssize_t how_many = load_random_buffer(256);
	unsigned char test_buf_a[256];
	test_buf_a[0] = 0;
	test_buf_a[1] = 126;
	test_buf_a[2] = 0;
	test_buf_a[3] = 126;
	memcpy(&test_buf_a[4], random_buffer, 126);
	ws_frame_append_chunk(p_frame, test_buf_a, 130);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert_int_eq(p_frame->frame_in, 130);
	ck_assert_int_eq(p_frame->payload_len, 126);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 126) == 0);
}
END_TEST

START_TEST(test_ws_frame_buf_127)
{
	ssize_t how_many = load_random_buffer(256);
	unsigned char test_buf_a[256];
	test_buf_a[0] = 0;
	test_buf_a[1] = 126;
	test_buf_a[2] = 0;
	test_buf_a[3] = 127;
	memcpy(&test_buf_a[4], random_buffer, 127);
	ws_frame_append_chunk(p_frame, test_buf_a, 131);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert_int_eq(p_frame->frame_in, 131);
	ck_assert_int_eq(p_frame->payload_len, 127);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 127) == 0);
}
END_TEST

START_TEST(test_ws_frame_buf_65535)
{
	ssize_t how_many = load_random_buffer(65535);
	unsigned char test_buf_a[65535 + 256];
	test_buf_a[0] = 0;
	test_buf_a[1] = 126;
	test_buf_a[2] = 0xFF;
	test_buf_a[3] = 0xFF;
	memcpy(&test_buf_a[4], random_buffer, 65535);
	ws_frame_append_chunk(p_frame, test_buf_a, 65535+4);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert_int_eq(p_frame->frame_in, 65535+4);
	ck_assert_int_eq(p_frame->payload_len, 65535);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 65535) == 0);
}
END_TEST

START_TEST(test_ws_frame_buf_65536)
{
	uint64_t copied;
	ssize_t how_many = load_random_buffer(65536);
	unsigned char test_buf_a[65536 + 256];
	ck_assert(how_many == 65536);
	test_buf_a[0] = 0;
	test_buf_a[1] = 127;
	test_buf_a[2] = 0;
	test_buf_a[3] = 0;
	test_buf_a[4] = 0;
	test_buf_a[5] = 0;
	test_buf_a[6] = 0;
	test_buf_a[7] = 1;
	test_buf_a[8] = 0;
	test_buf_a[9] = 0;
	memcpy(&test_buf_a[10], random_buffer, 65536);
	copied = ws_frame_append_chunk(p_frame, test_buf_a, 65536+10);
	ck_assert(copied == 65536+10);
	ck_assert_int_eq(p_frame->frame_in, 65536+10);
	ck_assert(p_frame->p_frame != NULL);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 65536) == 0);
	ck_assert_int_eq(p_frame->payload_len, 65536);
}
END_TEST

START_TEST(test_ws_frame_buf_65537)
{
	uint64_t copied;
	ssize_t how_many = load_random_buffer(65537);
	unsigned char test_buf_a[65536 + 256];
	ck_assert(how_many == 65537);
	test_buf_a[0] = 0;
	test_buf_a[1] = 127;
	test_buf_a[2] = 0;
	test_buf_a[3] = 0;
	test_buf_a[4] = 0;
	test_buf_a[5] = 0;
	test_buf_a[6] = 0;
	test_buf_a[7] = 1;
	test_buf_a[8] = 0;
	test_buf_a[9] = 1;
	memcpy(&test_buf_a[10], random_buffer, 65537);
	copied = ws_frame_append_chunk(p_frame, test_buf_a, 65537+10);
	ck_assert(copied == 65537+10);
	ck_assert_int_eq(p_frame->frame_in, 65537+10);
	ck_assert(p_frame->p_frame != NULL);
	ck_assert(ws_frame_is_valid(p_frame) != 0);
	ck_assert(memcmp(p_frame->p_payload, random_buffer, 65537) == 0);
	ck_assert_int_eq(p_frame->payload_len, 65537);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("WS_FRAME");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_ws_frame_ctor);
	tcase_add_test(tc_core, test_ws_frame_buf_124);
	tcase_add_test(tc_core, test_ws_frame_buf_125);
	tcase_add_test(tc_core, test_ws_frame_buf_126);
	tcase_add_test(tc_core, test_ws_frame_buf_127);
	tcase_add_test(tc_core, test_ws_frame_buf_65535);
	tcase_add_test(tc_core, test_ws_frame_buf_65536);
	tcase_add_test(tc_core, test_ws_frame_buf_65537);
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
	srunner_set_log (sr, "ws_frame.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

