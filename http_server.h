#ifndef __HTTP_SERVER__
#define __HTTP_SERVER__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <setjmp.h>
#include <signal.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Default file permissions are DEF_MODE & ~DEF_UMASK */
/* $begin createmasks */
#define DEF_MODE   S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH
#define DEF_UMASK  S_IWGRP|S_IWOTH
/* $end createmasks */

/* Simplifies calls to bind(), connect(), and accept() */
/* $begin sockaddrdef */
typedef struct sockaddr SA;
/* $end sockaddrdef */

/* Misc constants */
#define	MAXLINE	 1024  /* Max text line length */
#define MAXBUF   (1 << 14)  /* Max I/O buffer size */

#define HTTP 1
#define HTTPS 2

#define HTTP_PORT 8008
#define HTTPS_PORT 443

#define BLOGS_SIZE 16

#define DEFAULT_HOME "index.html"
#define HDR_RANGE "Range"

// 定义函数指针类型
typedef int (*ReadFunc)(int, void*, size_t);
typedef int (*WriteFunc)(int, void*, size_t);
struct Http_action {
    int range_tag;
    int rg_start;
    int rg_end;
};

/* http action func in http.c */
extern void request_handle(int fd, int http_type);
extern void parse_hdrs(int fd, ReadFunc do_read, struct Http_action *http_action);
extern void action_match(char *buf, struct Http_action *http_action);
extern int parse_uri(char *uri, char *filename);
extern void response_construct(int fd, char *filename, int filesize, WriteFunc do_write, struct Http_action *http_action);
extern void ok_response(int fd, char *filename, char *filetype, int filesize, char *success_code, WriteFunc do_write);
extern void get_filetype(char *filename, char *filetype);
extern void clienterror(int fd, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg, WriteFunc do_write);

/* Socket and IO func in server.c */
void sigchld_handler(int sig);
int do_socket(int __domain, int __type, int __protocol);
int do_setsockopt (int __fd, int __level, int __optname,
		       const void *__optval, socklen_t __optlen);
void srvaddr_init (struct sockaddr_in *srvaddr);
void do_bind(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
void do_listen(int __fd, int __blog);
void do_close(int fd);
int http_read(int fd, void *buf, size_t count);
int http_write(int fd, void *bufp, size_t n);
ssize_t do_readlineb(int fd, void *usrbuf, int maxlen, ReadFunc do_read);

#endif