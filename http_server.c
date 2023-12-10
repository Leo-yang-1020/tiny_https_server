/*
 * http_server.c - A simple, iterative HTTP/HTTPS Web server that uses the 
 *     GET method to serve static content.
 *
 */
#include "http_server.h"

int main(int argc, char **argv) 
{
    server_start(HTTP);
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
    srvaddr_init(&srvaddr);
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
            request_handle(connfd, HTTP);                   
            do_close(connfd);
            printf("conndfd %d closed\n", connfd);
            exit(0);
        }
        else if (pid > 0){
            do_close(connfd);
        }
    }
}

