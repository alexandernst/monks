#ifndef HOOKFNS_H_INCLUDED
#define HOOKFNS_H_INCLUDED

#include "syshijack.h"

typedef struct counter_info {
	atomic_t counter;
	char *name;
} __attribute__((packed)) counter_info_t;

extern counter_info_t __start_counters[];
extern counter_info_t __stop_counters[];

/*****************************************************************************************\
| HOOK MACROS                                                                             |
| __NR_#F, real_sys_##F and hooked_sys_##F stand for:                                     |
| __NR_#F = FUNCTION as defined in include/linux/syscalls.h                               |
| real_sys_##F = REAL FUNCTION as in the function in which we will save __NR_##F          |
| hooked_sys_##F = FAKE FUNCTION as in the function which we'll be using to fake __NR_##F |
\*****************************************************************************************/

#define HOOK(F)													\
	DEBUG(KERN_INFO "HOOK __NR_" #F "\n");						\
	real_sys_##F = (void *)sys_call_table[__NR_##F];			\
	sys_call_table[__NR_##F] = (void *)hooked_sys_##F;

#define UNHOOK(F)												\
	DEBUG(KERN_INFO "UNHOOK __NR_" #F "\n");					\
	sys_call_table[__NR_##F] = (void *)real_sys_##F;

#define REGISTER_SYSCALL(F)										\
	static counter_info_t __counter_info___NR_##F				\
	__attribute((unused, section(".counters"))) = {				\
		.counter = ATOMIC_INIT(0),								\
		.name = "__NR_" #F,										\
	};

#define __INCR(F)	\
	atomic_inc(&__counter_info___NR_##F.counter);

#define __DECR(F)	\
	atomic_dec(&__counter_info___NR_##F.counter);

#ifdef CONFIG_IA32_EMULATION

#define HOOK_IA32(F)											\
	DEBUG(KERN_INFO "HOOK_IA32 __NR32_" #F "\n");				\
	real_sys32_##F = (void *)ia32_sys_call_table[__NR32_##F];	\
	ia32_sys_call_table[__NR32_##F] = (void *)hooked_sys32_##F;

#define UNHOOK_IA32(F)											\
	DEBUG(KERN_INFO "UNHOOK_IA32 __NR32_" #F "\n");				\
	ia32_sys_call_table[__NR32_##F] = (void *)real_sys32_##F;

#define REGISTER_SYSCALL32(F)									\
	static counter_info_t __counter_info___NR32_##F				\
	__attribute((unused, section(".counters"))) = {				\
		.counter = ATOMIC_INIT(0),								\
		.name = "__NR32_" #F "_32",								\
	};

#define __INCR32(F)	\
	atomic_inc(&__counter_info___NR32_##F.counter);

#define __DECR32(F)	\
	atomic_dec(&__counter_info___NR32_##F.counter);

#endif /* CONFIG_IA32_EMULATION */

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void hook_calls(void);
void unhook_calls(void);

#endif