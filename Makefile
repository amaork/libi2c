CC			=	$(CROSS)gcc
AR			=	$(CROSS)ar
CPPFLAGS	=	-Wall -g
LDSHFLAGS	=	-rdynamic -shared 
ARFLAGS		=	rcv
TARGETS		=	libi2c.a libi2c.so

.PHONY:all clean

all:$(TARGETS)

clean:
	$(RM) *.o *~ depend $(TARGETS)

libi2c.a:i2c.o
	$(AR) $(ARFLAGS) $@ $^

libi2c.so:i2c.o
	$(CC) $(LDSHFLAGS) -o $@ $^

depend:$(wildcard *.c *.h)
	$(CC) $(CPPFLAGS) -MM $^ > $@

-include depend
