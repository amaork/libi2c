PYTHON		= python
CC			= $(CROSS)gcc
AR			= $(CROSS)ar
CFLAGS		= -Wall -g
LDSHFLAGS	= -rdynamic -shared 
ARFLAGS		= rcv
CODE_STYLE	= astyle --align-pointer=name --align-reference=name --suffix=none --break-blocks --pad-oper --pad-header --break-blocks --keep-one-line-blocks --indent-switches --indent=tab=4

SOURCES=$(filter-out src/pyi2c.c, $(wildcard src/*.c))
HEADERS=$(wildcard src/*.h)
OBJECTS=$(SOURCES:.c=.o)
TARGETS = libi2c.a libi2c.so pylibi2c.so

.PHONY:all clean example test install help style
.SILENT: clean

all:$(TARGETS) example

clean:
	make -C example clean
	find . -name "*.o" | xargs rm -f 
	$(RM) *.o *.so *~ a.out depend $(TARGETS) build -rf

help:
	$(PYTHON) help.py

test:
	$(PYTHON) -m unittest discover tests 

style:
	@find -regex '.*/.*\.\(c\|cpp\|h\)$$' | xargs $(CODE_STYLE)

install:
	$(PYTHON) setup.py install

example:$(TARGETS)
	make -C $@

libi2c.a:$(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

libi2c.so:$(OBJECTS)
	$(CC) $(LDSHFLAGS) -o $@ $^

pylibi2c.so:$(OBJECTS)
	$(PYTHON) setup.py build_ext --inplace

depend:$(SOURCES) $(HEADERS) src/pyi2c.c
	$(CC) $(CFLAGS) -MM $^ > $@

-include depend
