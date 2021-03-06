	
SRCFOLDER= src/
TESTFOLDER= test/
SOURCES= $(SRCFOLDER)child.o \
		 $(SRCFOLDER)signal.o \
		 $(SRCFOLDER)debug.o \
		 $(SRCFOLDER)avl.o \
		 $(SRCFOLDER)bst.o \
		 $(SRCFOLDER)logging.o 

MAINSRC= $(SOURCES) $(SRCFOLDER)main.o
TESTSRC= $(SOURCES) $(TESTFOLDER)test.o

CFLAGS= -D_POSIX_SOURCE=200112L \
		-std=c99 \
		-Wall \
		-Iinclude 
	#	-U_GNU_SOURCE \

LDFLAGS= -lpthread

# Name of our executable
EXE = server
TESTEXE = testserver

all: $(MAINSRC)
	gcc $(LDFLAGS) -o $(EXE) $(MAINSRC)

test: $(TESTSRC)
	gcc $(LDFLAGS) -o $(TESTEXE) $(TESTSRC)

#%.o: %.c
#	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $(TARGET_ARCH)\
#		-c $(INPUT) -o $(OUTPUT)

src/main.o: $(SRCFOLDER)main.c
	gcc $(CFLAGS) -c -o $(SRCFOLDER)main.o $(SRCFOLDER)main.c


test/test.o: $(TESTFOLDER)test.c
	gcc $(CFLAGS) -c -o $(TESTFOLDER)test.o $(TESTFOLDER)test.c


clean:
	@echo "Removing $(EXE)..."
	-rm $(EXE) &>/dev/null
	@echo "Removing $(TESTEXE)..."
	-rm $(TESTEXE) &>/dev/null
	@echo "Removing all *.o files...\n"
	-rm $(SRCFOLDER)*.o &>/dev/null
	-rm $(TESTFOLDER)*.o &>/dev/null 



