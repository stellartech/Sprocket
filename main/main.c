
#include <stdio.h>

static const char data[] =
    "GET /connect/0123456789012345678901234567891 HTTP/1.1\r\n"
    "Host: github.com\r\n"
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

int main(int argc, char** argv) 
{
	printf("Hello World\n");
	return 0;
	int is_upgrade = 0;
	size_t rval = 0;
	char *p_json_dump = NULL;
	json_t *p_request = NULL;

	p_request = json_object();
	rval = htparse_websocket_upgrade_request(data, data_len, p_request, &is_upgrade, 0);
	p_json_dump = json_dumps(p_request, JSON_INDENT(4));
	fprintf(stdout, "Is upgrade = %d\nData size = %d\nRval = %d\nJson: %s\n", is_upgrade, (int)data_len, (int)rval, p_json_dump);
	free(p_json_dump);
	json_decref(p_request);
	return 0;	
}

