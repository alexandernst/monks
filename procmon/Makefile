CC		=	gcc
CFLAGS	=	-g -Wall -Wextra -Werror

LDLIBS	=	$(shell pkg-config --silence-errors --libs libkmod ncurses)
OBJS	=	../common/mem_ops.o ../common/deserialize.o ui.o procmon-viewer.o
SOURCES	=	procmon-viewer.c
DESTS	=	../procmon-viewer

KMOD	=	$(shell pkg-config --exists libkmod && echo "1" || echo "0")

new: clean all

clean:
	@rm -rf *.o */*.o ../common/*.o $(DESTS)

all: _check_kmod $(DESTS)

_check_kmod:
ifeq ($(KMOD), 0)
CFLAGS += -D__NO_KMOD__
else
OBJS += lkm.o
endif

../procmon-viewer: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)
