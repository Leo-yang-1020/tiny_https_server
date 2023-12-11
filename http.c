#include "http_server.h"
/*
 * request_handle - handle one HTTP request/response transaction
 */
void request_handle(int fd, SSL *ssl, int http_type) 
{

    struct stat sbuf;
    struct Http_action http_action;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE];

    ReadFunc do_read;
    WriteFunc do_write;

    http_action.range_tag = 0; /* Initialized to be 0 */

    if (HTTP == http_type) {  // different http mode acts differently
        do_read = http_read;
        do_write = http_write;
    }
    else if (HTTPS == http_type) {
        do_read = https_read;
        do_write = https_write;
    }
    else {
        perror("unknown http type");
        exit(1);
    }
    /* Read request line and headers */
    do_readlineb(fd, ssl, buf, MAXBUF, do_read);
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    parse_hdrs(fd, ssl, do_read, &http_action);

    if (strcasecmp(method, "GET")) {          
        not_implement_response(fd, ssl, method, "501", do_write); // todo, currently only get supported
        return;
    }

    /* Parse URI from GET request */
    parse_uri(uri, filename);

    if (http_type == HTTP) {
        relocate_response(fd, ssl, uri, http_action.host, "301", do_write);
        return;
    }

    if (stat(filename, &sbuf) < 0) {                 
        not_found_response(fd, ssl, filename, "404", do_write);  // file not found error
        return;
    }            
       
	if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode)) {
	    forbidden_response(fd, ssl, filename, "403", do_write);   // no access to target file
	    return;
	}

	response_construct(fd, ssl, filename, sbuf.st_size, do_write, &http_action);
}


/*
 * parse_hdrs - read HTTP request headers
 */
void parse_hdrs(int fd, SSL *ssl, ReadFunc do_read, struct Http_action *http_action)
{
    char buf[MAXBUF];

    while(strcmp(buf, "\r\n")) {
        do_readlineb(fd, ssl, buf, MAXBUF, do_read);
        printf("%s",buf);
        action_match(buf, http_action);
    }
    return;
}

void action_match(char *buf, struct Http_action *http_action)
{
    int start = -1, end = -1;
    char host[MAXBUF];
    if (!strncmp(buf, HDR_HOST, strlen(HDR_HOST))) {
        sscanf(buf, "Host: %s", host);
        strncpy(http_action->host, host, sizeof(host));
        printf("%s\n", http_action->host);
    }
    else if (!strncmp(buf, HDR_RANGE, strlen(HDR_RANGE))) {
        http_action->range_tag = 1;
        sscanf(buf, "Range: bytes=%d-%d", &start, &end);
        printf("start:%d end:%d\n", start, end);
        http_action->rg_start = start;
        http_action->rg_end = end;
    }
}


/*
 * parse_uri 
 * 
 */
int parse_uri(char *uri, char *filename)
{
	strcpy(filename, ".");                      
	strcat(filename, uri);       
	if (uri[strlen(uri)-1] == '/')
	    strcat(filename, DEFAULT_HOME);
	return 0;
}

/*
 * response_construct - copy a file back to the client
 *   * Range supported: file from given start to end if range tag has been set
 */
void response_construct(int fd, SSL *ssl,char *filename, int filesize, WriteFunc do_write, struct Http_action *http_action)
{
    int srcfd;
    int start, end;
    int offset = 0;
    char *srcp, filetype[MAXLINE], buf[MAXBUF];
    srcfd = open(filename, O_RDONLY, 0);
    if (http_action->range_tag == 1) {
        ok_response(fd, ssl, filename, filetype, filesize, "206", do_write);
        // do file read limit by range
        start = http_action->rg_start;
        end = http_action->rg_end;
        if (start >= filesize) {
            perror("incorrect range");
            exit(1);
        }
        if (end == -1) {
            filesize -= start;
        }
        else {
            filesize = end - start + 1;
        }
        offset = start;
    }
    else {
        ok_response(fd, ssl, filename, filetype, filesize, "200", do_write);
    }
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    do_close(srcfd);
    /* Send response body to client */
    do_write(fd, ssl, srcp + offset, filesize);
    munmap(srcp, filesize);
}
/* 50X response */
void not_implement_response(int fd, SSL *ssl, char *method, char *code, WriteFunc do_write)
{
    clienterror(fd, ssl, method, code, "Not Implemented",  // only GET method supported
                   "does not implement this method", do_write);
}
/* 404 response */
void not_found_response(int fd, SSL *ssl, char *filename, char *code, WriteFunc do_write)
{
    clienterror(fd, ssl, filename, "404", "Not found",
                "Couldn't find this file", do_write);
}
/* 403 response */
void forbidden_response(int fd, SSL *ssl, char *filename, char *code, WriteFunc do_write)
{
    clienterror(fd, ssl, filename, "403", "Forbidden",
                "Couldn't access this file", do_write);
}

/* 30X response */
void relocate_response(int fd, SSL *ssl, char *uri, char *host, char *code, WriteFunc do_write)
{
    char buf[MAXBUF];
    sprintf(buf, "HTTP/1.0 %s relocate\r\n", code);
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "Location: https://%s%s\r\n", host, uri); /* Relocate http to https by default */
    do_write(fd, ssl, buf, strlen(buf));
}

/*
 * get_filetype - derive file type from file name
 */
void get_filetype(char *filename, char *filetype)
{
    if (strstr(filename, ".html"))
	strcpy(filetype, "text/html");
    else if (strstr(filename, ".gif"))
	strcpy(filetype, "image/gif");
    else if (strstr(filename, ".png"))
	strcpy(filetype, "image/png");
    else if (strstr(filename, ".jpg"))
	strcpy(filetype, "image/jpeg");
    else
	strcpy(filetype, "text/plain");
}

/* 
 *  Ok response for 2XX status code 
 */
void ok_response(int fd, SSL *ssl, char *filename, char *filetype, int filesize, char *success_code, WriteFunc do_write)
{
   /* Send response headers to client */
    char buf[MAXBUF];
    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 %s OK\r\n", success_code);
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "Server: Tiny Web Server\r\n");
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "Content-length: %d\r\n", filesize);
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "Content-type: %s\r\n\r\n", filetype);
    do_write(fd, ssl, buf, strlen(buf));
}

/*
 * clienterror - returns an error message to the client
 */
/* $begin clienterror */
void clienterror(int fd, SSL *ssl, char *cause, char *errnum, 
		 char *shortmsg, char *longmsg, WriteFunc do_write) 
{
    char buf[MAXLINE];

    /* Print the HTTP response headers */
    sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "Content-type: text/html\r\n\r\n");
    do_write(fd, ssl, buf, strlen(buf));

    /* Print the HTTP response body */
    sprintf(buf, "<html><title>Error</title>");
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "<body bgcolor=""ffffff"">\r\n");
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "%s: %s\r\n", errnum, shortmsg);
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "<p>%s: %s\r\n", longmsg, cause);
    do_write(fd, ssl, buf, strlen(buf));
    sprintf(buf, "<hr><em>Web server</em>\r\n");
    do_write(fd, ssl, buf, strlen(buf));
}
/* $end clienterror */
