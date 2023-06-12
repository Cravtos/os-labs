.PHONY: run build clean

run: build
	./server

build: server.c
	gcc server.c -o server

clean:
	rm server