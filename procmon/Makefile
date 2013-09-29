CC		=	gcc
CFLAGS	=	-g -Wall -Wextra -Werror
CFLAGS	+=	

LDLIBS	=	
LDLIBS	+=	

OBJS	=	../common/mem_ops.o ../common/deserialize.o procmon-viewer.o

SOURCES	=	procmon-viewer.c
DESTS	=	../procmon-viewer

new: clean all

clean:
	@rm -rf *.o */*.o ../common/*.o $(DESTS)

all: $(DESTS)

../procmon-viewer: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
