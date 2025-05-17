
/*socket_utils.h has the code that needs to change depending on the OS
code in /portable contains standard POSIX code (Linux) /riscos contains RISC OS specific code*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <swis.h>
#include <sys/socket.h>
#include <netinet/in.h> // Contains sockaddr_in definition
// #include <arpa/inet.h>   // For inet_addr()
#include <unistd.h> // For close()
#include <netdb.h>  // For gethostbyname()
#include <errno.h>
#include <sys/ioctl.h>
#include "socket_utils_wrapper.h" // For set_nonblocking()

#define PORT 12345
#define BUFFER_SIZE 1024

uint32_t manual_inet_addr(const char *ip_str); // Manual implementation of POSIX function inet_addr()

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    // int bytes_received;
    struct hostent *server;

    if (argc != 2)
    {
        printf("Usage: %s <server_ip>\n", argv[0]);
        return 1;
    }

    /* Create socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("socket() failed");
        return 1;
    }

    /*Set socket to non-blocking mode - this gets the right code depending on the OS selected (where?) */
    if (set_nonblocking(sock) == -1)
    {
        perror("Failed to set non-blocking");
        close(sock);
        return 1;
    }

    /* Set up server address */
    server = gethostbyname(argv[1]);
    if (server == NULL)
    {
        fprintf(stderr, "Error: Could not resolve hostname\n");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    // server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(PORT);

    /* ===== MANUAL IP CONVERSION ===== */
    server_addr.sin_addr.s_addr = manual_inet_addr(argv[1]);
    if (server_addr.sin_addr.s_addr == INADDR_NONE)
    {
        fprintf(stderr, "Invalid IP address: %s\n", argv[1]);
        close(sock);
        return 1;
    }

    /* Connect to server */
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        fprintf(stderr, "connect() failed: %s\n", strerror(errno));
        close(sock);
        return 1;
    }

    printf("Connected to server\n");

    /* Receive data */
    // bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    // if (bytes_received < 0)
    // {
    //     perror("recv");
    // }
    // else
    // {
    //     buffer[bytes_received] = '\0';
    //     printf("Received: %s", buffer);
    // }

    // Chat loop
    // while (1)
    // {
    //     /* Check for Backslash key */
    //     if (_kernel_osbyte(129, 0, 0) == 92)
    //     {
    //         printf("Escape pressed, shutting down...\n");
    //         break;
    //     }

    //     printf("> ");
    //     fgets(buffer, sizeof(buffer), stdin);

    //     if (send(sock, buffer, strlen(buffer), 0) < 0)
    //     {
    //         perror("Send() failed");
    //         break;
    //     }

    //     int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
    //     if (bytes_received <= 0)
    //     {
    //         perror("Server disconnected");
    //         break;
    //     }

    //     buffer[bytes_received] = '\0';
    //     printf("Server replied: %s", buffer);
    // }

    /* Chat loop */
    while (1)
    {
        /* Check for Backslash key (non-blocking) */
        char ch;
        if (read(STDIN_FILENO, &ch, 1) > 0 && ch == '\\')
        {
            printf("\nDisconnecting...\n");
            break;
        }

        /* Check for user input */
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        struct timeval tv = {0, 100000}; // 100ms timeout
        int activity = select(sock + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0)
        {
            perror("select error");
            break;
        }

        /* Handle server messages */
        if (FD_ISSET(sock, &readfds))
        {
            int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
            if (bytes_received <= 0)
            {
                if (bytes_received == 0)
                {
                    printf("Server disconnected\n");
                }
                else if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("recv error");
                }
                break;
            }
            buffer[bytes_received] = '\0';
            printf("Server: %s", buffer);
        }

        /* Handle user input */
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            if (fgets(buffer, sizeof(buffer), stdin) != NULL)
            {
                if (send(sock, buffer, strlen(buffer), 0) < 0)
                {
                    perror("send error");
                    break;
                }
            }
        }
    }

    /* Close connection */
    shutdown(sock, SHUT_RDWR);
    close(sock);

    return 0;
}

// Function to convert a string IP address to a 32-bit integer
// This is a manual implementation of inet_addr()
uint32_t manual_inet_addr(const char *ip_str)
{
    uint32_t addr = 0;
    char *end;
    int i;

    for (i = 0; i < 4; i++)
    {
        long val = strtoul(ip_str, &end, 10);
        if (val < 0 || val > 255 || (*end != '.' && *end != '\0' && i < 3))
        {
            return INADDR_NONE; // Invalid address
        }
        addr = (addr << 8) | (val & 0xFF);
        ip_str = end + 1;
    }
    return htonl(addr); // Convert to network byte order
}