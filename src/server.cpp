#include <arpa/inet.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static void make_request(int fd)
{
    char read_buf[64] = {};
    ssize_t n = read(fd, read_buf, sizeof(read_buf) - 1);
    if (n < 0)
    {
        msg("Failed to read fd");
        return;
    }
    fprintf(stderr, "Request: The client says %s\n", read_buf);

    char write_buf[] = "world";
    write(fd, write_buf, strlen(write_buf));
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);

    int value = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    // bind, this is the syntax that deals with IPv4 addresses
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;       // ipv4
    addr.sin_port = ntohs(8080);     // port 8080
    addr.sin_addr.s_addr = ntohl(0); // wildcard address 0.0.0.0

    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    {
        die("Failed to bind socket");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        die("Failed to create listener");
    }

    while (true)
    {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0)
        {
            continue;
        }
        make_request(connfd);
        close(connfd);
    }

    return 0;
    // int foo = bind(fd, 8080);
}
