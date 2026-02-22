#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <netinet/ip.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const size_t k_max_msg = 4096;

static void msg(const char *msg) { fprintf(stderr, "%s\n", msg); }

static void die(const char *msg)
{
    int err = errno;
    fprintf(stderr, "[%d] %s\n", err, msg);
    abort();
}

static int32_t read_all(int fd, char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = read(fd, buf, n);
        if (rv <= 0)
        {
            return -1; // error or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }

    return 0;
}

static int32_t write_all(int fd, const char *buf, size_t n)
{
    while (n > 0)
    {
        ssize_t rv = write(fd, buf, n);
        if (rv <= 0)
        {
            return -1; // error or unexpected EOF
        }
        assert((size_t)rv <= n);
        n -= (size_t)rv;
        buf += rv;
    }
    return 0;
}

static int32_t reqwest(int fd)
{
    // 4 bytes header
    char rbuf[4 + k_max_msg + 1] = {};
    errno = 0;
    int32_t err = read_all(fd, rbuf, 4);

    if (err)
    {
        if (errno == 0)
        {
            msg("EOF");
        }
        else
        {
            msg("Failed to read fd");
        }
        return err;
    }

    uint32_t len = 0;
    memcpy(&len, rbuf, 4); // assuming little endian mode
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }

    // request body
    err = read_all(fd, &rbuf[4], len);
    if (err)
    {
        msg("failed to read fd");
        return err;
    }

    // do something
    rbuf[4 + len] = '\0';
    printf("client says: %s\n", &rbuf[4]);

    // reply using the same protocol
    const char reply[] = "world";
    char wbuf[4 + sizeof(reply)];
    len = (uint32_t)strlen(reply);

    memcpy(wbuf, &len, 4);
    memcpy(&wbuf[4], reply, len);
    return write_all(fd, wbuf, 4 + len);
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

        while (true)
        {
            int32_t err = reqwest(connfd);
            if (err)
            {
                break;
            }
        }

        close(connfd);
    }

    return 0;
}
