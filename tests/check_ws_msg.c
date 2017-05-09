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

#define WS_FRAG_FRIEND
#include "../src/ws_frag.h"

#define WS_MSG_FRIEND
#include "../src/ws_msg.h"

#include "ws_frag_maker.h"

void setup(void)
{
}

void teardown(void)
{
}

typedef struct {
	uint64_t test_size;
	char mask[4];
} test_item_t;

#define MASKED 'a','b','c','d'

// We don't fully test all the way up as it takes a long time
// for the tests to complete. Uncomment this if you are debugging
// really high buffer counts.
//#define FULL_WS_FRAME_TEST

test_item_t test_items[] =
{
	  { 0x00000100UL,       { 0,0,0,0 } } // 4 
	, { 0x00001000UL,       { 0,0,0,0 } } // 4 
	, { 0x00010000UL,       { 0,0,0,0 } } // 4 
	, { 0x00100000UL,       { 0,0,0,0 } } // 4 
};

START_TEST(test_ws_msg_looped)
{
	int offset1, offset2;
	unsigned char *p_mask = test_items[_i].mask;
	uint64_t used, copied, test_size = test_items[_i].test_size;
	ws_msg_pt p_local = ws_msg_ctor();
	unsigned char *p_msg;
	unsigned char *p_test_data1 = ws_make_test_buffer(test_size, 0x03, test_items[_i].mask, &offset1);
	unsigned char *p_test_data2 = ws_make_test_buffer(test_size, 0x80, test_items[_i].mask, &offset2);

	copied =  ws_msg_append_chunk(p_local, p_test_data1, test_size + offset1);
	copied += ws_msg_append_chunk(p_local, p_test_data2, test_size + offset2);
	ck_assert(copied == (test_size*2) + offset1 + offset2);

	ck_assert(ws_msg_is_valid(p_local) != 0);

	copied = (test_size*2) + offset1 + offset2;
	used = ws_msg_memory_usage(p_local);
	ck_assert_msg(used == copied,
		"failed: size:%ld ne expected 0x%ld", used, copied);
	
	p_msg = ws_msg_pullup(p_local, &used);
	ck_assert_msg(p_msg, "p_msg is null :(");
	ck_assert(memcmp(p_msg, &p_test_data1[offset1], test_size) == 0);
	ck_assert(memcmp(p_msg+test_size, &p_test_data2[offset2], test_size) == 0);
	free(p_msg);

	buffer_pt p_buffer = ws_msg_pullup_as_buffer(p_local);
	ck_assert(memcmp(buffer_ptr(p_buffer), &p_test_data1[offset1], test_size) == 0);
	ck_assert(memcmp(buffer_ptr(p_buffer)+test_size, &p_test_data2[offset2], test_size) == 0);
	buffer_dtor(&p_buffer);
	ck_assert(!p_buffer);
	
	free(p_test_data1);
	free(p_test_data2);
	ws_msg_free(p_local);
}
END_TEST


Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("WS_MSG");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_loop_test(tc_core, test_ws_msg_looped, 
		0, (sizeof(test_items)/sizeof(test_item_t)));
	tcase_set_timeout(tc_core, 60);
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
	srunner_set_log (sr, "ws_msg.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

#if 0
#include <stdio.h>
static void
log_packet(unsigned char *inp, unsigned char *inp2) {
	FILE *fp = fopen("blob", "a+");
	if(fp) {
		for(int i = 0; i < 16; i++) {
			fprintf(fp, "%02x ", *(inp+i));
		}
		fprintf(fp, "\n");
		for(int i = 0; i < 16; i++) {
			fprintf(fp, "%02x ", *(inp+i));
		}
		fprintf(fp, "\n\n");
		fclose(fp);
	}

}
#endif

