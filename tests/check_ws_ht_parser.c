
#include <check.h>

#include "../src/ws_ht_parser.h"

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

static void
string_test(json_t *inp, const char *key, const char *val)
{
	ck_assert(json_is_object(inp));
	ck_assert(json_is_string(json_object_get(inp, key)));
	ck_assert_str_eq(val, json_string_value(json_object_get(inp, key)));
}

START_TEST(ws_ht_parser)
{
	int is_upgrade = 0;
	size_t rval = 0;
	char *p_json_dump = NULL;
	const char *p_string;
	json_t *p = NULL, *p_test = NULL;
	json_error_t j_err;

	p = json_object();
	rval = htparse_websocket_upgrade_request(data, data_len, p, &is_upgrade, 0);
	p_json_dump = json_dumps(p, 0);
	json_decref(p); // Destroy original object.

	p = json_loads(p_json_dump, 0, &j_err); // And reload it via json string.
	free(p_json_dump); // Now destroy teh temp string.

	// Begin tests of the json object.
	string_test(p, "URI", "/connect/0123456789012345678901234567891");
	string_test(p, "Host", "example.com");
	string_test(p, "Upgrade", "WebSocket");
}
END_TEST

Suite *suite()
{
	Suite *s;
	TCase *tc_core;
	s = suite_create("WS_HT_PARSER");
	tc_core = tcase_create("Core");
	tcase_add_test(tc_core, ws_ht_parser);
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
	srunner_run_all(sr, CK_NORMAL);
	number_failed = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

