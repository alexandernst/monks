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
# TODO: add unistd_32.h autogeneration
#       if CONFIG_IA32_EMULATION enabled
#

_prebuild: unistd_32.h
