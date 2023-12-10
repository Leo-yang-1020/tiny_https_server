/*
 *  Basic socket function supported
 *     More robust IO and network function wrapper
 */
#include "http_server.h"

void sigchld_handler(int sig)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}

int do_socket(int __domain, int __type, int __protocol)
{
    int socket_fd;
    socket_fd = socket(__domain, __type, __protocol);
    if(socket_fd < 0) {
        perror("socket create error");
        exit(1);
    }
    return socket_fd;
}

int do_setsockopt (int __fd, int __level, int __optname,
		       const void *__optval, socklen_t __optlen)
{
    if (setsockopt(__fd, __level, __optname, __optval, __optlen) < 0) {
        perror("setsockopt error");
        exit(1);
    }
}

void srvaddr_init (struct sockaddr_in *srvaddr)
{
    bzero(srvaddr, sizeof(srvaddr));
    srvaddr->sin_family = AF_INET;
    srvaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr->sin_port = htons(HTTP_PORT);
}

void do_bind(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len)
{
    if (bind(__fd, __addr, __len) < 0) {
        perror("bind error");
        exit(1);
    }
}

void do_listen(int __fd, int __blog)
{
    if (listen(__fd, __blog) < 0) {
        perror("listen error");
        exit(1);
    }
}

void do_close(int fd)
{
    if (close(fd) < 0) {
        perror("close error");
        exit(1);
    }
}

int http_read(int fd, void *buf, size_t count)
{
    int sz;
    if ((sz = read(fd, buf, count)) < 0) {
        perror("read error");
        exit(1);
    }
    return sz;
}

int http_write(int fd, void *bufp, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)
		nwritten = 0;
	    else
		perror("do_write_all error");
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}

ssize_t do_readlineb(int fd, void *usrbuf, int maxlen, ReadFunc do_read)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = do_read(fd, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}