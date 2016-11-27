CROSS		=	
CC			= $(CROSS)gcc
AR			= $(CROSS)ar
CFLAGS		= -Wall -g
LDSHFLAGS	= -rdynamic -shared 
ARFLAGS		= rcv

SOURCES=$(filter-out src/pyi2c.c, $(wildcard src/*.c))
HEADERS=$(wildcard src/*.h)
OBJECTS=$(SOURCES:.c=.o)
TARGETS = libi2c.a libi2c.so pylibi2c.so

.PHONY:all clean test
.SILENT: clean

all:$(TARGETS) test

clean:
	make -C test clean
	find . -name "*.o" | xargs rm -f 
	$(RM) *.o *~ a.out depend $(TARGETS) build -rf

test:$(TARGETS)
	make -C test

libi2c.a:$(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libi2c.so:$(OBJECTS)
	$(CC) $(LDSHFLAGS) -o $@ $^

pylibi2c.so:
	python setup.py build_ext --inplace

depend:$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $^ > $@

-include depend
