CC = gcc
CFLAGS = -Wextra -g
LDFLAGS =

TARGET = http_server
SRCS = http_server.c http.c server.c
OBJS = $(SRCS:.c=.o)
DEPS = http_server.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@
	rm -f $(OBJS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
