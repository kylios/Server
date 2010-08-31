include ../Makefile

EXE= test

all: $(SOURCES) test.o linktests
#
#test.o: test.c
#	gcc -c test.c
#
linktests:
	gcc $(LDFLAGS) -o $(EXE) $(SOURCES)

