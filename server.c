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

void srvaddr_init(struct sockaddr_in *srvaddr, int port)
{
    bzero(srvaddr, sizeof(srvaddr));
    srvaddr->sin_family = AF_INET;
    srvaddr->sin_addr.s_addr = htonl(INADDR_ANY);
    srvaddr->sin_port = htons(port);
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

int http_read(int fd, SSL *ssl, void *buf, size_t count)
{
    int sz;
    if ((sz = read(fd, buf, count)) < 0) {
        perror("read error");
        exit(1);
    }
    return sz;
}

int http_write(int fd, SSL *ssl, void *bufp, size_t n)
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

int https_read(int fd, SSL *ssl, void *buf, size_t count)
{
    int sz;
    if ((sz = SSL_read(ssl, buf, count)) < 0) {
        perror("ssl read error");
        exit(1);
    }
    return sz;
}

int https_write(int fd, SSL *ssl, void *buf, size_t n)
{
    size_t nleft = n;
    ssize_t nwritten;

    while (nleft > 0) {
	if ((nwritten = SSL_write(ssl, buf, nleft)) <= 0) {
	    if (errno == EINTR)
		nwritten = 0;
	    else
		perror("ssl write error");
	}
	nleft -= nwritten;
	buf += nwritten;
    }
    return n;
}

ssize_t do_readlineb(int fd, SSL *ssl, void *usrbuf, int maxlen, ReadFunc do_read)
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = do_read(fd, ssl, &c, 1)) == 1) {
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

SSL * SSL_conn_build(int fd)
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

    if (SSL_CTX_use_certificate_file(ctx, CERT_PATH, SSL_FILETYPE_PEM) <= 0) {
        // public key
        perror("load public key error");
        exit(1);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, PRIKEY_PATH, SSL_FILETYPE_PEM) <= 0) {
        // private key
        perror("load private key error");
        exit(1);
    }
    if (SSL_CTX_check_private_key(ctx) <= 0) {
        perror("check private key error");
        exit(1);
    }

    SSL *ssl = SSL_new(ctx);

    if(ssl == NULL) {
        perror("SSL_new error");
        exit(1);
    }

    if(SSL_set_fd(ssl, fd) == 0) {
        perror("SSL_set_fd error");
        exit(1);
    }
    return ssl;
}