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

#define __HOOK(F, RF, FF)									\
do{															\
	DEBUG(KERN_INFO "HOOK " #F "\n");						\
	RF = (void *)sys_call_table[F];							\
	sys_call_table[F] = (void *)FF;							\
	REGISTER(F);											\
}while(0)

#define HOOK(name)											\
	__HOOK(__NR_##name, real_sys_##name, hooked_sys_##name)

#define __UNHOOK(F, RF)										\
do{															\
	DEBUG(KERN_INFO "UNHOOK " #F "\n");						\
	sys_call_table[F] = (void *)RF;							\
}while(0)

#define UNHOOK(name)										\
	__UNHOOK(__NR_##name, real_sys_##name)

#ifdef CONFIG_IA32_EMULATION

#define __HOOK_IA32(F, RF, FF)								\
do{															\
	DEBUG(KERN_INFO "HOOK_IA32 " #F "\n");					\
	RF = (void *)ia32_sys_call_table[F];					\
	ia32_sys_call_table[F] = (void *)FF;					\
	REGISTER(F);											\
}while(0)

#define HOOK_IA32(name)										\
	__HOOK_IA32(__NR32_##name, real_sys32_##name, hooked_sys32_##name)

#define __UNHOOK_IA32(F, RF)								\
do{															\
	DEBUG(KERN_INFO "UNHOOK_IA32 " #F "\n");				\
	ia32_sys_call_table[F] = (void *)RF;					\
}while(0)

#define UNHOOK_IA32(name)									\
	__UNHOOK_IA32(__NR32_##name, real_sys32_##name)

#endif /* CONFIG_IA32_EMULATION */

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void hook_calls(void);
void unhook_calls(void);

#endif