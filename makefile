CC=gcc
CFLAGS=-Wall -g
LIBS=-lsqlite3

todo: todo.o
	$(CC) -o todo todo.o $(LIBS)

todo.o: todo.c
	$(CC) $(CFLAGS) -c todo.c

clean:
	rm -f todo todo.o