#ifndef DESERIALIZE_H_INCLUDED
#define DESERIALIZE_H_INCLUDED

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

#include "mem_ops.h"
#include "structures.h"

syscall_info *deserialize_syscall_info(membuffer *buffer);
void *get_chunk(membuffer *buffer);

#endif