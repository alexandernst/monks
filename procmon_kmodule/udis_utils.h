#ifndef UDIS_UTILS_H_INCLUDED
#define UDIS_UTILS_H_INCLUDED

#include "msgs.h"
#include "../udis86/udis86.h"

uint64_t ud_get_stub_size(void *entry);
void ud_patch_addr(void *entry, void *addr);
unsigned int ud_find_insn_arg(void *entry, int limit, enum ud_mnemonic_code insn_mne, int insn_len);

#endif