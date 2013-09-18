NAME		:= procmon
KDIR		?= /lib/modules/$(shell uname -r)/build

ccflags-y	:= -g -Wall
ldflags-y	:= -T$(src)/sections.lds

obj-m		:= $(NAME).o
obj-y		:= syscalls/

$(NAME)-y	:= main.o sct_hook.o utils.o \
		   syscalls/built-in.o

all: _prebuild
	make -C $(KDIR) M=$(shell pwd)

clean:
	make -C $(KDIR) M=$(shell pwd) clean

#
# TODO: check for CONFIG_IA32_EMULATION
#

_prebuild: unistd_32.h

unistd_32.h: /usr/include/asm/unistd_32.h
	sed s/__NR_/__NR32_/g /usr/include/asm/unistd_32.h > unistd_32.h

clean-files	+= unistd_32.h