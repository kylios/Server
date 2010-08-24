	
SRCFOLDER= src/
SOURCES= $(SRCFOLDER)child.o \
		 $(SRCFOLDER)signal.o \
		 $(SRCFOLDER)main.o \
		 $(SRCFOLDER)debug.o \
		 $(SRCFOLDER)logging.o 
CFLAGS= -D_POSIX_SOURCE=200112L \
		-std=c99 \
		-Wall \
		-Iinclude 
	#	-U_GNU_SOURCE \

LDFLAGS= -lpthread

# Name of our executable
EXE = server

all: $(SOURCES) link

#%.o: %.c
#	$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDES) $(TARGET_ARCH)\
#		-c $(INPUT) -o $(OUTPUT)

link:
	gcc $(LDFLAGS) -o $(EXE) $(SOURCES)

clean:
	@echo "Removing $(EXE)..."
	-rm $(EXE) &>/dev/null
	@echo "Removing all *.o files...\n"
	-rm -R src/*.o &>/dev/null


