#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>

#include "structures.h"

char *serialize_membuffer(membuffer *buffer);
membuffer *serialize_syscall_info(syscall_info *i);
void add_chunk(membuffer *buffer, void *chunk, size_t size);

#endif