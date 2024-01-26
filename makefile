CC=gcc
CFLAGS=-Wall -g -I/usr/local/include
LIBS=-lsqlite3 -lraylib -framework IOKit -framework Cocoa -framework OpenGL -framework Metal

todo: todo.o
	$(CC) -o todo todo.o $(LIBS)

todo.o: todo.c
	$(CC) $(CFLAGS) -c todo.c

clean:
	rm -f todo todo.o