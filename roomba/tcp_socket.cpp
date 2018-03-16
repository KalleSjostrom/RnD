#ifdef OS_WINDOWS
#include "winsock2.h"
#include "ws2tcpip.h"

b32 make_socket(i32 &socket_handle) {
	socket_handle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	b32 success = socket_handle >= 0;
	if (!success) {
		LOG_ERROR("TCPSocket", "Could not create socket! (error=%ld)\n", GetLastError());
	}
	return success;
}
b32 setup_host_socket(i32 socket_handle, u16 port, u32 max_pending_connections) {	
	ASSERT(socket_handle >= 0, "Ivalid socket handle.");
	ASSERT(max_pending_connections <= 128, "Too many pending connections.");

	sockaddr_in address = {};

	address.sin_family      = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port        = htons(port);

	b32 success = bind(socket_handle, (sockaddr*) &address, sizeof(address)) >= 0;
	if (!success) {
		LOG_ERROR("TCPSocket", "Could not bind socket! (error=%ld)\n", GetLastError());
		return false;
	}

	success = listen(socket_handle, 8) >= 0;
	if (!success) {
		LOG_ERROR("TCPSocket", "listen() failed! (error=%ld)\n", GetLastError());
	}

	return success;
}

b32 setup_client_socket(i32 socket_handle, const char *ip, u16 port) {
	ASSERT(socket_handle >= 0, "Ivalid socket handle.");

	sockaddr_in address = {};
	address.sin_family = PF_INET;
	inet_pton(PF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	b32 success = connect(socket_handle, (sockaddr*) &address, sizeof(address)) >= 0;
	if (!success) {
		LOG_ERROR("TCPSocket", "Could not connect to server! (error=%ld)\n", GetLastError());
	}
	u_long nonzero = 1;
	ioctlsocket(socket_handle, FIONBIO, &nonzero);
	return success;
}

b32 accept_client(i32 socket_handle, i32 &accepted_socket) {
	struct sockaddr_in client_address; /* Client address */
	i32 client_address_length = (i32)sizeof(client_address);
	accepted_socket = accept(socket_handle, (sockaddr *) &client_address, &client_address_length);
	b32 success = accepted_socket >= 0;
	if (!success) {
		LOG_ERROR("TCPSocket", "accept() failed (error=%ld)\n", GetLastError());
	}
	return success;
}

i32 recieve_socket(i32 socket_handle, u8 *buffer, u32 count) {
	i32 recieved_size = recv(socket_handle, (char*)buffer, count, MSG_PEEK);
	if (recieved_size > 0) {
		recieved_size = recv(socket_handle, (char*)buffer, count, 0);
	}
	return (i32) recieved_size;
}

i32 send_socket(i32 socket_handle, u8 *buffer, u32 count) {
	i32 sent_size = send(socket_handle, (char*)buffer, count, 0);
	if (sent_size == -1) {
		printf("Failed to send\n");
	}
	return (i32) sent_size;
}

void close_socket(i32 &socket_handle) {
	_close(socket_handle);
	socket_handle = -1;
}
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

b32 make_socket(i32 &socket_handle) {
	socket_handle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	b32 success = socket_handle >= 0;
	if (!success) {
		fprintf(stderr, "Could not create socket!\n");
	}
	return success;
}

b32 setup_host_socket(i32 socket_handle, u32 port, u32 max_pending_connections) {
	ASSERT(socket_handle >= 0, "Ivalid socket handle.");
	ASSERT(max_pending_connections <= 128, "Too many pending connections.");

	sockaddr_in address = {};

	address.sin_family      = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port        = htons(port);

	b32 success = bind(socket_handle, (sockaddr*) &address, sizeof(address)) >= 0;
	if (!success) {
		fprintf(stderr, "Could not bind socket!\n");
		return false;
	}

	success = listen(socket_handle, 8) >= 0;
	if (!success) {
		fprintf(stderr, "listen() failed!\n");
	}

	return success;
}

b32 setup_client_socket(i32 socket_handle, const char *ip, u32 port) {
	ASSERT(socket_handle >= 0, "Ivalid socket handle.");

	sockaddr_in address = {};

	address.sin_family      = PF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port        = htons(port);

	b32 success = connect(socket_handle, (sockaddr*) &address, sizeof(address)) >= 0;
	if (!success) {
		fprintf(stderr, "Could not connect to server! %d\n", errno);
	}
	return success;
}

b32 accept_client(i32 socket_handle, i32 &accepted_socket) {
	struct sockaddr_in client_address; /* Client address */
	u32 client_address_length = sizeof(client_address);
	accepted_socket = accept(socket_handle, (sockaddr *) &client_address, &client_address_length);
	b32 success = accepted_socket >= 0;
	if (!success) {
		fprintf(stderr, "accept() failed\n");
	}
	return success;
}

i32 recieve_socket(i32 socket_handle, u8 *buffer, u32 count) {
	ssize_t recieved_size = recv(socket_handle, buffer, count, 0);
	return (i32) recieved_size;
}

i32 send_socket(i32 socket_handle, u8 *buffer, u32 count) {
	ssize_t sent_size = send(socket_handle, buffer, count, 0);
	if (sent_size == -1) {
		printf("Failed to send\n");
	}
	return (i32) sent_size;
}

void close_socket(i32 &socket_handle) {
	close(socket_handle);
	socket_handle = -1;
}
#endif