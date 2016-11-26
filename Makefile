CROSS		=	
CC			= $(CROSS)gcc
AR			= $(CROSS)ar
CFLAGS		= -Wall -g
LDSHFLAGS	= -rdynamic -shared 
ARFLAGS		= rcv

SOURCES=$(wildcard src/*.c)
HEADERS=$(wildcard src/*.h)
OBJECTS=$(SOURCES:.c=.o)
TARGETS = libi2c.a libi2c.so

.PHONY:all clean test
.SILENT: clean

all:$(TARGETS) test

clean:
	make -C test clean
	find . -name "*.o" | xargs rm -f 
	$(RM) *.o *~ a.out depend $(TARGETS) -f

test:$(TARGETS)
	make -C test

libi2c.a:$(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libi2c.so:$(OBJECTS)
	$(CC) $(LDSHFLAGS) -o $@ $^

depend:$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $^ > $@

-include depend
