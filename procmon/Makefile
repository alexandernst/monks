CC		=	@gcc
CFLAGS	=	-g -Wall -w -Wextra -Werror
CFLAGS	+=	

LDLIBS	=	../common/deserialize.o
LDLIBS	+=	

SOURCES	=	procmon-user.c
DESTS	=	$(patsubst %.c,%,$(SOURCES))

new: clean all

clean:
	@rm -rf *.o */*.o $(DESTS)

all: $(DESTS)