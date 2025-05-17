
#include "riscos/socket_utils_riscos.h"
#include <sys/ioctl.h>

int set_nonblocking_riscos(int sockfd) {
    int on = 1;
    return ioctl(sockfd, FIONBIO, &on);
}