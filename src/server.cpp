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
    char rbuf[64] = {};
    ssize_t n = read(fd, rbuf, sizeof(rbuf) - 1);
    if (n < 0)
    {
        msg("Failed to read fd");
        return;
    }
    fprintf(stderr, "[REQUEST]: client says %s\n", rbuf);

    char wbuf[] = "world";
    write(fd, wbuf, strlen(wbuf));
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("failed to create socket fd");
    }

    int value = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

    // bind, this is the syntax that deals with IPv4 addresses
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;       // ipv4
    addr.sin_port = ntohs(8080);     // port 8080
    addr.sin_addr.s_addr = ntohl(0); // address 0.0.0.0

    int rv = bind(fd, (const struct sockaddr *)&addr, sizeof(addr));
    if (rv)
    {
        die("Failed to bind to socket fd");
    }

    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        die("Failed to listen to socket fd");
    }

    while (true)
    {
        struct sockaddr_in client_addr = {};
        socklen_t socklen = sizeof(client_addr);
        int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
        if (connfd < 0)
        {
            continue; // there was an error so skip and try again
        }

        make_request(connfd);
        close(connfd);
    }

    return 0;
}
