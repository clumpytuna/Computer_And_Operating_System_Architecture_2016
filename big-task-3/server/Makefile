include ../config

all: server

main.c: read_map-2.h safe_read-2.h find_way-2.h variable.h

server: main.c
	$(CC) $(CFLAGS) -o $@ $^  -lpthread

clean:
	rm -f server


