#ifdef OS_WINDOWS
#include "winsock2.h"
#include "ws2tcpip.h"

bool make_socket(SOCKET *socket_handle) {
	*socket_handle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (*socket_handle == INVALID_SOCKET) {
		LOG_ERROR("TCPSocket", "Could not create socket! (error=%ld)\n", GetLastError());
	}
	return *socket_handle != INVALID_SOCKET;
}
bool setup_host_socket(SOCKET socket_handle, uint16_t port, uint32_t max_pending_connections) {	
	assert(socket_handle != INVALID_SOCKET && "Ivalid socket handle.");
	assert(max_pending_connections <= 128 && "Too many pending connections.");

	SOCKADDR_IN address;

	address.sin_family      = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port        = htons(port);

	int success = bind(socket_handle, (SOCKADDR*) &address, sizeof(address)) >= 0;
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

bool setup_client_socket(SOCKET socket_handle, const char *ip, uint16_t port) {
	assert(socket_handle != INVALID_SOCKET);

	SOCKADDR_IN address;
	address.sin_family = PF_INET;
	inet_pton(PF_INET, ip, &address.sin_addr);
	address.sin_port = htons(port);

	bool success = connect(socket_handle, (SOCKADDR*) &address, sizeof(address)) >= 0;
	if (!success) {
		LOG_ERROR("TCPSocket", "Could not connect to server! (error=%ld)\n", GetLastError());
	}
	u_long nonzero = 1;
	ioctlsocket(socket_handle, FIONBIO, &nonzero);
	return success;
}

bool accept_client(int socket_handle, SOCKET *accepted_socket) {
	struct sockaddr_in client_address; /* Client address */
	int client_address_length = (int)sizeof(client_address);
	*accepted_socket = accept(socket_handle, (SOCKADDR *) &client_address, &client_address_length);
	bool success = *accepted_socket != INVALID_SOCKET;
	if (!success) {
		LOG_ERROR("TCPSocket", "accept() failed (error=%ld)\n", GetLastError());
	}
	return success;
}

int recieve_socket(SOCKET socket_handle, uint8_t *buffer, uint32_t count) {
	int recieved_size = recv(socket_handle, (char*)buffer, count, MSG_PEEK);
	if (recieved_size > 0) {
		recieved_size = recv(socket_handle, (char*)buffer, count, 0);
	}
	return (int) recieved_size;
}

int send_socket(SOCKET socket_handle, uint8_t *buffer, uint32_t count) {
	int sent_size = send(socket_handle, (char*)buffer, count, 0);
	if (sent_size == -1) {
		printf("Failed to send\n");
	}
	return (int) sent_size;
}

void close_socket(SOCKET socket_handle) {
	closesocket(socket_handle);
	socket_handle = INVALID_SOCKET;
}
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

bool make_socket(int &socket_handle) {
	socket_handle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	bool success = socket_handle >= 0;
	if (!success) {
		fprintf(stderr, "Could not create socket!\n");
	}
	return success;
}

bool setup_host_socket(int socket_handle, uint32_t port, uint32_t max_pending_connections) {
	assert(socket_handle >= 0 && "Ivalid socket handle.");
	assert(max_pending_connections <= 128 && "Too many pending connections.");

	sockaddr_in address = {};

	address.sin_family      = PF_INET;
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	address.sin_port        = htons(port);

	bool success = bind(socket_handle, (SOCKADDR*) &address, sizeof(address)) >= 0;
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

bool setup_client_socket(int socket_handle, const char *ip, uint32_t port) {
	assert(socket_handle >= 0 && "Ivalid socket handle.");

	sockaddr_in address = {};

	address.sin_family      = PF_INET;
	address.sin_addr.s_addr = inet_addr(ip);
	address.sin_port        = htons(port);

	bool success = connect(socket_handle, (SOCKADDR*) &address, sizeof(address)) >= 0;
	if (!success) {
		fprintf(stderr, "Could not connect to server! %d\n", errno);
	}
	return success;
}

bool accept_client(int socket_handle, int &accepted_socket) {
	struct sockaddr_in client_address; /* Client address */
	uint32_t client_address_length = sizeof(client_address);
	accepted_socket = accept(socket_handle, (SOCKADDR *) &client_address, &client_address_length);
	bool success = accepted_socket >= 0;
	if (!success) {
		fprintf(stderr, "accept() failed\n");
	}
	return success;
}

int recieve_socket(int socket_handle, uint8_t *buffer, uint32_t count) {
	ssize_t recieved_size = recv(socket_handle, buffer, count, 0);
	return (int) recieved_size;
}

int send_socket(int socket_handle, uint8_t *buffer, uint32_t count) {
	ssize_t sent_size = send(socket_handle, buffer, count, 0);
	if (sent_size == -1) {
		printf("Failed to send\n");
	}
	return (int) sent_size;
}

void close_socket(int &socket_handle) {
	close(socket_handle);
	socket_handle = -1;
}
#endif