#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "socket_wrapper.h"

int create_socket(int domaine, int type, int protocole, const char* error_msg){
  int socket_fd = socket(domaine, type, protocole);
  if(socket_fd == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
  printf("socket created successfully\n");

  return socket_fd;
}

struct sockaddr_in init_sockaddr(sa_family_t family, in_port_t port, uint32_t s_addr){
  struct sockaddr_in adr;
  adr.sin_family = family;
  adr.sin_port = htons(port);
  adr.sin_addr.s_addr = s_addr;

  printf("socket address initialized successfully\n");
  return adr;
}

void bind_socket(int socket_fd, struct sockaddr* adr, socklen_t size, const char* error_msg){
  int bind_res = bind(socket_fd, adr, size);

  if(bind_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  printf("socket address bound successfully\n");
}
void listen_socket(int socket_fd, int backlog, const char* error_msg){
  int listen_res = listen(socket_fd, backlog);
  if(listen_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

void connect_socket(int socket_fd, const struct sockaddr *adr, socklen_t size, const char* error_msg){
  int connect_res = connect(socket_fd, adr, size);
  if(connect_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

int accept_socket(int socket_fd, struct sockaddr *adr, socklen_t *size, const char* error_msg){
  printf("server listening and awaiting connections...\n");
  int client_socket_fd = accept(socket_fd, adr, size);
  if(client_socket_fd <= 0){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }

  return client_socket_fd;
}

void close_socket(int socket_fd, const char* error_msg){
  int close_res = close(socket_fd);
  if(close_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

void send_message(int socket_fd, const void *buffer, size_t buffer_size, int permissions, const char* error_msg){
  int send_res = send(socket_fd, buffer, buffer_size, permissions);
  if(send_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}

void recv_message(int socket_fd, void *buffer, size_t buffer_size, int permissions, const char* error_msg){
  int recv_res = recv(socket_fd, buffer, buffer_size, permissions);
  if(recv_res == -1){
    perror(error_msg);
    exit(EXIT_FAILURE);
  }
}
