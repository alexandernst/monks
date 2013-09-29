NAME		:= procmon
KDIR		?= /lib/modules/$(shell uname -r)/build

ccflags-y	:= -g -Wall
ldflags-y	:= -T$(src)/sections.lds

obj-m		:= $(NAME).o
obj-y		:= syscalls/

$(NAME)-y	:= 	main.o sct_hook.o utils.o \
		   		syscalls/built-in.o \
		   		../common/mem_ops.o ../common/serialize.o

new: clean all

clean:
	make -C $(KDIR) M=$(shell pwd) clean

all: _prebuild
	make -C $(KDIR) M=$(shell pwd)

_prebuild: unistd_32.h

unistd_32.h:
	@echo "#include <asm/unistd_32.h>" | $(CC) -E -dM - | grep __NR_ | sed s/__NR_/__NR32_/g > $@

clean-files	+= unistd_32.h