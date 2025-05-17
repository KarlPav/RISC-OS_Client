
#include "posix/socket_utils_posix.h"
#include <fcntl.h>
#include <unistd.h>

int set_nonblocking_posix(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1)
        return -1;
    return fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
}