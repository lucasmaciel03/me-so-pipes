CC = gcc
CFLAGS = -Wall -Wextra -O2

all: build/server build/client

build/server: src/server.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

build/client: src/client.c
	@mkdir -p build
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -rf build/* logs/* /tmp/exec_fifo /tmp/log_fifo_*
	