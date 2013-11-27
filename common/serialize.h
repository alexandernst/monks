#ifndef SERIALIZE_H_INCLUDED
#define SERIALIZE_H_INCLUDED

#include "mem_ops.h"
#include "structures.h"

membuffer *serialize_syscall_info(syscall_intercept_info *i);
int add_chunk(membuffer *buffer, void *chunk, size_t size);

#endif