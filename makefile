CC = gcc
CFLAGS = -Wall -Wextra
LIBS = -lpthread

all: server client

server: server.o
	$(CC) $(CFLAGS) -o server server.o $(LIBS)

client: client.o
	$(CC) $(CFLAGS) -o client client.o $(LIBS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f server client *.o
