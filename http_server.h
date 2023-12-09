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
#define MAXBUF   1024  /* Max I/O buffer size */
#define LISTENQ  1024  /* Second argument to listen() */


// 定义函数指针类型
typedef int (*do_read)(int, void*, size_t);

// read1函数的实现
int read1(int fd, void* buf, size_t count) {
    // 实现read1的逻辑
    printf("Using read1 function\n");
    return 0;
}

// read2函数的实现
int read2(int fd, void* buf, size_t count) {
    // 实现read2的逻辑
    printf("Using read2 function\n");
    return 0;
}


#endif