
#include "riscos/socket_utils_riscos.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <kernel.h>

int set_nonblocking_riscos(int sockfd)
{
    int on = 1;
    return ioctl(sockfd, FIONBIO, &on);
}

#include <kernel.h>
#include <swis.h>


int riscos_stdin_ready() {
    _kernel_swi_regs regs;
    regs.r[0] = 0x13;  // OS_Byte 13 (check keyboard buffer)
    _kernel_swi(OS_Byte, &regs, &regs);
    return (regs.r[1] != 0);  // 1 = key(s) pending
}