#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

int main()
{

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("failed to create socket fd");
    }

    // bind, this is the syntax that deals with IPv4 addresses
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;                     // ipv4
    addr.sin_port = ntohs(8080);                   // port 8080
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // address 127.0.0.1

    int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("Failed to connect to socket fd");
    }

    char message[] = "hello";
    write(fd, message, strlen(message));

    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        die("Failed to read fd");
    }

    printf("[RESPONSE]: server says: %s\n", rbuf);
    close(fd);

    return 0;
}
