
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

#define PORT 12345
#define BUFFER_SIZE 256

uint32_t manual_inet_addr(const char *ip_str); //Manual implementation of POSIX function inet_addr()

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int bytes_received;
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
        return 1;
    }

    printf("Connected to server\n");

    /* Receive data */
    bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0)
    {
        perror("recv");
    }
    else
    {
        buffer[bytes_received] = '\0';
        printf("Received: %s", buffer);
    }

    // Chat loop
    while (1)
    {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);

        if (send(sock, buffer, strlen(buffer), 0) < 0)
        {
            perror("Send() failed");
            break;
        }

        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0)
        {
            perror("Server disconnected");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("Server replied: %s", buffer);
    }

    /* Close connection */
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