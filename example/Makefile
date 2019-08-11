CC			= $(CROSS)gcc
AR			= $(CROSS)ar
CFLAGS		= -Wall -g
LDSHFLAGS	= -rdynamic -shared 
ARFLAGS		= rcv
CFLAGS		+= -I../src
LDFLAGS		= -L.. -li2c -Wl,-R -Wl,..

OBJDIR=../objs
SOURCES = $(wildcard *.c) $(wildcard *.cpp)
TARGETS = $(foreach src, $(SOURCES), $(basename $(src))) 

.PHONY:all clean objdir
.SILENT:clean

all:objdir $(TARGETS)

objdir:
	@mkdir -p $(OBJDIR)

clean:
	$(RM) *.o a.out depend $(TARGETS) $(OBJDIR) -rf

i2c_tools: i2c_tools.o
	$(CC) $(CFLAGS) -o $(OBJDIR)/$@ $^ $(LDFLAGS)

i2c_without_internal_address: i2c_without_internal_address.o
	$(CC) $(CFLAGS) -o $(OBJDIR)/$@ $^ $(LDFLAGS)

depend:$(wildcard *.h *.c)
	$(CC) $(CFLAGS) -MM $^ > $@

-include depend
