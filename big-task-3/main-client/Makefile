include ../config


all: client


client: main_client.c
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -f client
