CC=gcc
CFLAGS=-Wall -c -DIS_RPI -DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -g -ftree-vectorize -pipe -Wno-psabi -mcpu=cortex-a53 -mfloat-abi=hard -mfpu=neon-fp-armv8 -mneon-for-64bits $(shell pkg-config --cflags gtk+-3.0) -Wno-deprecated-declarations
LDFLAGS=-Wall -o -Wl,--whole-archive -Wl,--no-whole-archive -rdynamic $(shell pkg-config --cflags gtk+-3.0) -lpthread -lrt -ldl -lm $(shell pkg-config --libs gtk+-3.0) -lasound $(shell pkg-config --libs gtk+-3.0) $(shell pkg-config --libs sqlite3)
SOURCES=SQLiteBrowser.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=SQLiteBrowser

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o
	-rm -f $(EXECUTABLE)
