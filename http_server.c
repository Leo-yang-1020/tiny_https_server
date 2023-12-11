/*
 * http_server.c - A simple, iterative HTTP/HTTPS Web server that uses the 
 *     GET method to serve static content.
 *
 */
#include "http_server.h"
int server_start(int http_type);
int http_server(int connfd);
int https_server(int connfd);

int main(int argc, char **argv)
{
    int pid;
    pid = fork();
    if (pid == 0)
        server_start(HTTP); /* One process for Http server, port 80 specifically */
    else
        server_start(HTTPS); /* One process for Https server, port 80 specifically */
}

int server_start(int http_type)
{
    int listenfd, connfd;
    int optval = 1;
    int pid;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_in srvaddr,cliaddr;

    signal(SIGCHLD, sigchld_handler);
    listenfd = do_socket(AF_INET, SOCK_STREAM, 0);
    do_setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    srvaddr_init(&srvaddr, http_type == HTTP? HTTP_PORT: HTTPS_PORT);
    do_bind(listenfd, (struct sockaddr *)&srvaddr, sizeof(srvaddr));
    do_listen(listenfd, BLOGS_SIZE);

    while (1) {
        clientlen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clientlen);
        getnameinfo((struct sockaddr *) &cliaddr, clientlen, hostname, MAXLINE, 
                        port, MAXLINE, 0);
        printf("Accepted connection from (%s, %s), connfd %d\n", hostname, port, connfd);
        pid = fork();
        if (pid == 0) {
            if (HTTP == http_type)
                http_server(connfd);
            else if (HTTPS == http_type)
                https_server(connfd);
            exit(0);
        }
        else if (pid > 0) {
            do_close(connfd);
        }
    }
}

int http_server(int connfd)
{
    request_handle(connfd, NULL, HTTP);                   
    do_close(connfd);
    printf("conndfd %d closed\n", connfd);
}

int https_server(int connfd)
{
    SSL *ssl;
    ssl = SSL_conn_build(connfd);
    if (SSL_accept(ssl) == -1) {
        ERR_print_errors_fp(stderr);
    }
    request_handle(connfd, ssl, HTTPS);
    do_close(connfd);
    SSL_free(ssl);
}

