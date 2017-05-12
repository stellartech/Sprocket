


#include <stdio.h>

#define FRIEND_OF_SERVER_CONN
#include "../src/tcp_server/server_conn.h"

int main(int argc, char** argv) 
{
	printf("Hello World %ld %016x\n", sizeof(server_conn_t), 1234);
	return 0;	
}

