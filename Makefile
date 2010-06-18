CFLAGS=-g -Wall
SOURCES=server.o redblack.o

all: $(SOURCES)
	gcc $(CFLAGS) $(SOURCES) -levent -o server 

clean:
	rm -f *.o server
