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

#include "../src/base64.h"


static unsigned char test_string[] = "This quick brown fox";
static unsigned char encoded_string[] = "VGhpcyBxdWljayBicm93biBmb3g=";

void setup(void)
{
}

void teardown(void)
{
}

START_TEST(test_base64_encode)
{
	int encoded_len = 0; 
	unsigned char *encoded = base64_encode(test_string, strlen(test_string), &encoded_len);
	ck_assert_str_eq(encoded, encoded_string);
	ck_assert_int_eq(encoded_len, strlen(encoded_string));
	free((void*)encoded);
}
END_TEST

START_TEST(test_base64_decode)
{
	unsigned char *decoded = NULL;
	decoded = base64_decode(encoded_string, strlen(encoded_string));
	ck_assert_str_eq(test_string, decoded);
	free((void*)decoded);
}
END_TEST

START_TEST(test_base64_io)
{
	int encoded_len = 0; 
	unsigned char *actual, *encoded;
	encoded = base64_encode(test_string, strlen(test_string), &encoded_len);
	actual = base64_decode(encoded, encoded_len);
	ck_assert_str_eq(encoded, encoded_string);
	free((void*)encoded);
	free((void*)actual);
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;

	s = suite_create("BASE64");

	tc_core = tcase_create("Core");
	tcase_add_checked_fixture(tc_core, setup, teardown);
	tcase_add_test(tc_core, test_base64_encode);
	tcase_add_test(tc_core, test_base64_decode);
	tcase_add_test(tc_core, test_base64_io);
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
	srunner_set_log (sr, "base64.log");
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

