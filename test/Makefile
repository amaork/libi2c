CROSS		=	
CC			= $(CROSS)gcc
AR			= $(CROSS)ar
CFLAGS		= -Wall -g
LDSHFLAGS	= -rdynamic -shared 
ARFLAGS		= rcv
CFLAGS		+=	-I../src
LDFLAGS		= -L.. -li2c -Wl,-R -Wl,..

OBJDIR=../objs
TARGETS=i2c_test

.PHONY:all clean objdir
.SILENT:clean

all:objdir $(TARGETS)

objdir:
	@mkdir -p $(OBJDIR)

clean:
	$(RM) *.o a.out depend $(TARGETS) $(OBJDIR) -rf

i2c_test: i2c_test.o
	$(CC) $(CFLAGS) -o $(OBJDIR)/$@ $^ $(LDFLAGS)

depend:$(wildcard *.h *.c)
	$(CC) $(CFLAGS) -MM $^ > $@

-include depend
