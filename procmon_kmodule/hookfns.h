#ifndef HOOKFNS_H_INCLUDED
#define HOOKFNS_H_INCLUDED

#include "syshijack.h"

extern raw_spinlock_t _sl;

void hook_calls(void);
void unhook_calls(void);

#endif