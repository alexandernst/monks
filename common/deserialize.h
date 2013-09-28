#ifndef DESERIALIZE_H_INCLUDED
#define DESERIALIZE_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "structures.h"

membuffer *deserialize_membuffer(char *data);
syscall_info *deserialize_syscall_info(membuffer *buffer);
void *get_chunk(membuffer *buffer);

#endif