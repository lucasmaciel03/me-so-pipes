CC = gcc
CFLAGS = -Wall -Wextra -02

all: build/server build/client

build/server: src/server.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

build/client: src/client.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf build/* logs/* /temp/exec_fifo /tmp/log_fifo_*
	