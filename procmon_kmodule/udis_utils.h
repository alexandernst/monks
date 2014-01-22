#ifndef UDIS_UTILS_H_INCLUDED
#define UDIS_UTILS_H_INCLUDED

#include "../udis86/udis86.h"

typedef struct addrs_t {
	int len1, len2;
	void *addr1, *addr2;
} addrs_t;

uint64_t ud_get_stub_size(void *entry);
void ud_patch_addr(void *entry, void *addr);
void ud_patch_cmp(void *entry);
void *ud_find_syscall_table_addr(void *entry);

#endif