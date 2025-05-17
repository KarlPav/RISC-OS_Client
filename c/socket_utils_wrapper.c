
#include "socket_utils_wrapper.h"

// Include the correct platform header
#ifdef __riscos__
#include "riscos/socket_utils_riscos.h"
#else
#include "posix/socket_utils_posix.h"
#endif

// Single implementation that delegates to platform-specific function
int set_nonblocking(int sockfd)
{
#ifdef __riscos__
    return set_nonblocking_riscos(sockfd);
#else
    return set_nonblocking_posix(sockfd);
#endif
}