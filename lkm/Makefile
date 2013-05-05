EXTRA_CFLAGS := -std=gnu99

obj-m := procmon.o
procmon-y := syshijack.o hookfns.o

KVER := $(shell uname -r)
KDIR = /lib/modules/$(KVER)/build
UNAME := $(shell uname -m)

ifeq ($(UNAME), x86_64)
all: unistd_32.h
endif

all:
	@make -C $(KDIR) M=$(PWD) modules

unistd_32.h: /usr/include/asm/unistd_32.h
	@sed -e 's/__NR_/__NR32_/g' $< > $@

clean:
	@make -C $(KDIR) M=$(PWD) clean
	@rm unistd_32.h &>/dev/null || true