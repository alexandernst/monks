# -*- Makefile -*-

KDIR := /lib/modules/$(shell uname -r)/build

all modules:
	@make -C ${KDIR} M=$$PWD;

clean modules_install:
	@make -C ${KDIR} M=$$PWD $@;
	@rm unistd_32.h &>/dev/null || true