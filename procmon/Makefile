CC		=	gcc
CFLAGS	=	-g -Wall -Wextra -Werror
CFLAGS	+=	

LDLIBS	=	
LDLIBS	+=	

OBJS	=	../common/mem_ops.o ../common/deserialize.o procmon-user.o

SOURCES	=	procmon-user.c
DESTS	=	procmon-user

new: clean all

clean:
	@rm -rf *.o */*.o ../common/*.o $(DESTS)

all: $(DESTS)

procmon-user: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)
