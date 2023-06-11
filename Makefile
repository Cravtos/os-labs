.PHONY: server clean

run: server
	./server

server: server.c
	gcc server.c -o server

clean:
	rm server