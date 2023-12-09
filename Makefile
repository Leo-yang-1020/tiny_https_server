all: http_server

httpd: http_server.c
	gcc  -lpthread -o http_server http_server.c

clean:
	rm http_server
