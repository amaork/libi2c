CROSS		=	
PYTHON		= python
CC		= $(CROSS)gcc
AR		= $(CROSS)ar
CFLAGS		= -Wall -g
LDSHFLAGS	= -rdynamic -shared 
ARFLAGS		= rcv

SOURCES=$(filter-out src/pyi2c.c, $(wildcard src/*.c))
HEADERS=$(wildcard src/*.h)
OBJECTS=$(SOURCES:.c=.o)
TARGETS = libi2c.a libi2c.so pylibi2c.so

.PHONY:all clean example test install
.SILENT: clean

all:$(TARGETS) example

clean:
	make -C example clean
	find . -name "*.o" | xargs rm -f 
	$(RM) *.o *.so *~ a.out depend $(TARGETS) build -rf

test:
	$(PYTHON) -m unittest discover tests 

install:
	$(PYTHON) setup.py install

example:$(TARGETS)
	make -C $@

libi2c.a:$(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libi2c.so:$(OBJECTS)
	$(CC) $(LDSHFLAGS) -o $@ $^

pylibi2c.so:
	$(PYTHON) setup.py build_ext --inplace

depend:$(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -MM $^ > $@

-include depend
