# -*- Makefile -*-

KDIR := /lib/modules/$(shell uname -r)/build

all modules:
	@echo -ne "#ifndef EVERYTHING_H_INCLUDED\n#define EVERYTHING_H_INCLUDED\n\n" > syscalls/everything.h
	@ls -1 syscalls | grep -v "everything.h" | grep ".h$$" | awk '{print "#include ""\""$$1"\""}' >> syscalls/everything.h
	@echo -ne "\n#endif\n" >> syscalls/everything.h
	@make -C ${KDIR} M=$$PWD;

clean modules_install:
	@make -C ${KDIR} M=$$PWD $@;
	@rm syscalls/everything.h &>/dev/null || true
	@rm unistd_32.h &>/dev/null || true