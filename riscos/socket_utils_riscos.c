
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
#include <stdio.h>

// Returns:
//  1 if data is ready for fgets()
//  0 if no data available
// -1 on error
int riscos_stdin_ready()
{
    _kernel_swi_regs regs;
    regs.r[0] = 0x13; // OS_Byte 13 (check keyboard buffer)
    _kernel_swi(OS_Byte, &regs, &regs);
    return (regs.r[1] != 0) ? 1 : 0;
}

// Non-blocking fgets() wrapper
// Returns like standard fgets(), or NULL if no data
char *nb_fgets(char *buf, int size, FILE *stream)
{
    if (stream != stdin || !riscos_stdin_ready())
    {
        return NULL;
    }
    return fgets(buf, size, stdin);
}

int gen_func_riscos()
{
    // This function is a placeholder for any RISC OS-specific functionality.
    // It can be replaced with actual implementation as needed.
    return 0; // Return 0 to indicate success
}