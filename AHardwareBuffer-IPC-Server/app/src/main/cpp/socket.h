//
// Created by intel on 2/4/2023.
//
#ifndef AHARDWAREBUFFER_IPC_SERVER_SOCKET_H
#define AHARDWAREBUFFER_IPC_SERVER_SOCKET_H
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C"{
#endif
int create_socket(const char *path);
int connect_socket(int sock, const char *path);
int write_fd(int sock, int fd, void *data, size_t data_len);
int read_fd(int sock, int *fd, void *data, size_t data_len);
#ifdef  __cplusplus
}
#endif

#endif //AHARDWAREBUFFER_IPC_SERVER_SOCKET_H
