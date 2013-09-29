#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

#include "mem_ops.h"
#include "structures.h"

char *serialize_membuffer(membuffer *buffer);
membuffer *serialize_syscall_info(syscall_info *i);
void add_chunk(membuffer *buffer, void *chunk, size_t size);

#endif