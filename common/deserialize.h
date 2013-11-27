#ifndef DESERIALIZE_H_INCLUDED
#define DESERIALIZE_H_INCLUDED

#include "mem_ops.h"
#include "structures.h"

syscall_intercept_info *deserialize_syscall_info(membuffer *buffer);
void *get_chunk(membuffer *buffer);

#endif