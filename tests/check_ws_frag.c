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
	  { 0x00000001UL,       { 0,0,0,0 } } // 0
	, { 0x00000010UL,       { 0,0,0,0 } } // 1
	, { 0x00000010UL - 1,   { 0,0,0,0 } } // 2
	, { 0x00000010UL + 1,   { 0,0,0,0 } } // 3
	, { 0x00000100UL,       { 0,0,0,0 } } // 4 
	, { 0x00000100UL - 1,   { 0,0,0,0 } } // 5
	, { 0x00000100UL + 1,   { 0,0,0,0 } } // 6
	, { 0x00001000UL,       { 0,0,0,0 } } // 7
	, { 0x00001000UL - 1,   { 0,0,0,0 } } // 8
	, { 0x00001000UL + 1,   { 0,0,0,0 } } // 9
	, { 0x00010000UL,       { 0,0,0,0 } } // 10
	, { 0x00010000UL - 1,   { 0,0,0,0 } } // 11
	, { 0x00010000UL + 1,   { 0,0,0,0 } } // 12
#ifdef FULL_WS_FRAME_TEST
	, { 0x00100000UL,       { 0,0,0,0 } }
	, { 0x00100000UL - 1,   { 0,0,0,0 } }
	, { 0x00100000UL + 1,   { 0,0,0,0 } }
	, { 0x01000000UL,       { 0,0,0,0 } }
	, { 0x01000000UL - 1,   { 0,0,0,0 } }
	, { 0x01000000UL + 1,   { 0,0,0,0 } }
	, { 0x10000000UL,       { 0,0,0,0 } }
	, { 0x10000000UL - 1,   { 0,0,0,0 } }
	, { 0x10000000UL + 1,   { 0,0,0,0 } }
	, { 0x7FFFFFFFUL,       { 0,0,0,0 } }
#endif
	, { 0x00000001UL,       { MASKED } }
	, { 0x00000010UL,       { MASKED } }
	, { 0x00000010UL - 1,   { MASKED } }
	, { 0x00000010UL + 1,   { MASKED } }
	, { 0x00000100UL,       { MASKED } }
	, { 0x00000100UL - 1,   { MASKED } }
	, { 0x00000100UL + 1,   { MASKED } }
	, { 0x00001000UL,       { MASKED } }
	, { 0x00001000UL - 1,   { MASKED } }
	, { 0x00001000UL + 1,   { MASKED } }
	, { 0x00010000UL,       { MASKED } }
	, { 0x00010000UL - 1,   { MASKED } }
	, { 0x00010000UL + 1,   { MASKED } }
#ifdef FULL_WS_FRAME_TEST
	, { 0x00100000UL,       { MASKED } }
	, { 0x00100000UL - 1,   { MASKED } }
	, { 0x00100000UL + 1,   { MASKED } }
	, { 0x01000000UL,       { MASKED } }
	, { 0x01000000UL - 1,   { MASKED } }
	, { 0x01000000UL + 1,   { MASKED } }
	, { 0x10000000UL,       { MASKED } }
	, { 0x10000000UL - 1,   { MASKED } }
	, { 0x10000000UL + 1,   { MASKED } }
	, { 0x7FFFFFFFUL,       { MASKED } }
#endif
};

START_TEST(test_ws_frag_looped)
{
	int offset;
	unsigned char *p_mask = test_items[_i].mask;
	uint64_t copied, test_size = test_items[_i].test_size;
	ws_frag_pt p_local = ws_frag_ctor();
	unsigned char *p_test_data = ws_make_test_buffer(test_size, 0x83, test_items[_i].mask, &offset);
	copied = ws_frag_append_chunk(p_local, p_test_data, test_size + offset);
	ck_assert(copied == test_size + offset);
	ck_assert_int_eq(p_local->frag_in, test_size + offset);
	ck_assert(p_local->p_frag != NULL);
	ck_assert(ws_frag_is_valid(p_local) != 0);
	ck_assert_msg(p_local->payload.len == test_size, 
		"failed: 0x%lx ne expected 0x%lx", p_local->payload.len, test_size);
	if(p_mask[0]!=0||p_mask[1]!=0||p_mask[2]!=0||p_mask[3]!=0) {
		unsigned char *p = (unsigned char*)p_local->p_payload;
		for(int i = 0; i < test_size; i++) {
			p[i] ^= p_mask[(i & 0x3)];
		}
	}
	ck_assert(memcmp(p_local->p_payload, &p_test_data[offset], test_size) == 0);
	free(p_test_data);
	ws_frag_free(p_local);
}
END_TEST


Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("WS_FRAG");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_loop_test(tc_core, test_ws_frag_looped, 
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
	srunner_set_log (sr, "ws_frag2.log");
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

