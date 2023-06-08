CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lpthread

all: server client

server: server.c
	$(CC) $(CFLAGS) server.c -o server $(LIBS)

client: client.c
	$(CC) $(CFLAGS) client.c -o client $(LIBS)

clean:
	rm -f server client *.o
