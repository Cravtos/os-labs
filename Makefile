.PHONY: all monitor server clean

all: server monitor

monitor: monitor.c
	gcc monitor.c -o monitor -lpthread

server: server.c
	gcc server.c -o server

clean:
	rm server monitor