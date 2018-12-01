#ifndef _SOCKET_WRAPPER_H
#define _SOCKET_WRAPPER_H
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

/*FUNCTIONS*/
int create_socket(int domaine, int type, int protocole, const char* error_msg);
struct sockaddr_in init_sockaddr(sa_family_t family, in_port_t port, uint32_t s_addr);
void bind_socket(int socket_fd, struct sockaddr *adr, socklen_t size, const char* error_msg);
void listen_socket(int socket_fd, int backlog, const char* error_msg);
void connect_socket(int socket_fd, const struct sockaddr *adr, socklen_t size, const char* error_msg);
int accept_socket(int socket_fd, struct sockaddr *adr, socklen_t *size, const char* error_msg);
void close_socket(int socket_fd, const char* error_msg);
void send_message(int socket_fd, const void *buffer, size_t buffer_size, int permissions, const char* error_msg);
void recv_message(int socket_fd, void *buffer, size_t buffer_size, int permissions, const char* error_msg);

#endif // _SOCKET_WRAPPER_H
