CC		=	gcc
CFLAGS	=	-g -Wall -Wextra -Werror
CFLAGS	+=	$(shell pkg-config --silence-errors --cflags libkmod)

LDLIBS	=	$(shell pkg-config --silence-errors --libs libkmod)
LDLIBS	+=	

OBJS	=	../common/mem_ops.o ../common/deserialize.o lkm.o procmon-viewer.o

SOURCES	=	procmon-viewer.c
DESTS	=	../procmon-viewer

new: clean all

clean:
	@rm -rf *.o */*.o ../common/*.o $(DESTS)

all: $(DESTS)

../procmon-viewer: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
