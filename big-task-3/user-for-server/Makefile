include ../config


all: player

main.c: print_ending.h safe_read.h

player: main.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread


clean:
	rm -f player
