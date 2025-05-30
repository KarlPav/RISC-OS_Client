
#ifndef SOCKET_UTILS_POSIX_H
#define SOCKET_UTILS_POSIX_H

#include <fcntl.h>
#include <unistd.h>

int set_nonblocking_posix(int sockfd);
int gen_func_posix();

#endif