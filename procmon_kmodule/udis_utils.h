#ifndef UDIS_UTILS_H_INCLUDED
#define UDIS_UTILS_H_INCLUDED

#include "msgs.h"
#include "../udis86/udis86.h"

uint64_t ud_get_stub_size(void *entry);
void ud_patch_addr(void *entry, void *addr);
void ud_patch_cmp(void *entry);
void *ud_find_syscall_table_addr(void *entry);

#endif