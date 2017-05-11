
#include <stdio.h>
#include <unistd.h>

#include "../src/tcp_sock_server.h"


static void
acc(tcp_sock_server_pt inp_server, 
	int in_fd, 
	struct sockaddr *inp_sockaddr, 
	int in_sockaddr_len, 
	void* inp_userdata)
{
	ssize_t count;
	char buf[512];
	while(1) {
		count = read(in_fd, buf, sizeof(buf));
		if(count == 0 || count == -1) { close(in_fd); return; }
		write(1, buf, count);
	}
}

int main(int argc, char** argv) 
{
	tcp_sock_server_pt p_server = tcp_sock_server_ctor(NULL);

	tcp_sock_server_set_ip(p_server, "0.0.0.0");
	tcp_sock_server_set_port(p_server, 8080);
	tcp_sock_server_set_acc_cb(p_server, acc);
	tcp_sock_server_set_err_cb(p_server, acc);
	tcp_sock_server_bind(p_server);

	printf("Hello World\n");
	char c = getchar();

	tcp_sock_server_free(p_server);
	return 0;	
}

