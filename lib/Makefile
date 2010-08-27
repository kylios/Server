
SRCFOLDER= ./
SOURCES= $(SRCFOLDER)server.o 
CFLAGS= -D_POSIX_SOURCE=200112L \
		-std=c99 \
		-Wall \
		-I../include 
	#	-U_GNU_SOURCE \



test_server: server.o test_server.o debug.o
	gcc -o test_server server.o test_server.o debug.o

server.o: server.c server.h
	gcc $(CFLAGS) -c server.o server.c

test_server.o: test_server.c server.h
	gcc $(CFLAGS) -c test_server.o test_server.c

debug.o: ../include/debug.h ../src/debug.c
	gcc $(CFLAGS) -c debug.o ../src/debug.c
#
#clean:
#	-rm server.o &>/dev/null
#	rm test_server.o
#
