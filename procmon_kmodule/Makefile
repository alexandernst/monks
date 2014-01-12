NAME		:= procmon
KDIR		?= /lib/modules/$(shell uname -r)/build

ccflags-y	:= -g -Wall -nostdinc
ldflags-y	:= -T$(src)/sections.lds

obj-m		:= $(NAME).o
obj-y		:= syscalls/ ../udis86/libudis86/

$(NAME)-y := main.o control.o sct_hook.o netlink.o utils.o \
             udis_utils.o syscalls/built-in.o \
             ../common/mem_ops.o ../common/serialize.o

$(NAME)-y += ../udis86/libudis86/built-in.o \
             ../udis86/libudis86/syn.o \
             ../udis86/libudis86/syn-intel.o

$(NAME)-y	+= stubs.o

new: clean all

clean:
	make -C $(KDIR) M=$(shell pwd) clean
	@rm -rf ../common/*.o ../udis86/libudis86/*.o
	@rm -rf ../$(NAME).ko

all: _prebuild
	make -C $(KDIR) M=$(shell pwd)
	@mv $(NAME).ko ../$(NAME).ko

_prebuild: unistd_32.h

unistd_32.h:
	@echo "#include <asm/unistd_32.h>" | $(CC) -E -dM - | grep __NR_ | sed s/__NR_/__NR32_/g > $@

clean-files	+= unistd_32.h