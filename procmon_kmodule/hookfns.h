#ifndef HOOKFNS_H_INCLUDED
#define HOOKFNS_H_INCLUDED

#include "syshijack.h"

typedef struct counter_info {
	atomic_t counter;
	char *name;
} __attribute__((packed)) counter_info_t;

extern counter_info_t __start_counters[];
extern counter_info_t __stop_counters[];

/*****************************************************************************\
| HOOK MACROS                                                                 |
| F, RF and FF stand for:                                                     |
| F = FUNCTION as defined in include/linux/syscalls.h                         |
| RF = REAL FUNCTION as in the function in which we will save F               |
| FF = FAKE FUNCTION as in the function which we'll be using to fake F        |
\*****************************************************************************/

#define __HOOK(F, RF, FF)									\
	DEBUG(KERN_INFO "HOOK " #F "\n");						\
	RF = (void *)sys_call_table[F];							\
	sys_call_table[F] = (void *)FF;

#define REGISTER_SYSCALL(F)									\
	static counter_info_t __counter_info___NR_##F			\
	__attribute((unused, section(".counters"))) = {			\
		.counter = ATOMIC_INIT(0),							\
		.name = "__NR_" #F,									\
	};

#define HOOK(name)											\
	__HOOK(__NR_##name, real_sys_##name, hooked_sys_##name);

#define __UNHOOK(F, RF)										\
	DEBUG(KERN_INFO "UNHOOK " #F "\n");						\
	sys_call_table[F] = (void *)RF;

#define UNHOOK(name)										\
	__UNHOOK(__NR_##name, real_sys_##name)

#ifdef CONFIG_IA32_EMULATION

#define __HOOK_IA32(F, RF, FF)								\
	DEBUG(KERN_INFO "HOOK_IA32 " #F "\n");					\
	RF = (void *)ia32_sys_call_table[F];					\
	ia32_sys_call_table[F] = (void *)FF;

#define REGISTER_SYSCALL32(F)								\
	static counter_info_t __counter_info___NR32_##F			\
	__attribute((unused, section(".counters"))) = {			\
		.counter = ATOMIC_INIT(0),							\
		.name = "__NR32_" #F "_32",							\
	};

#define HOOK_IA32(name)										\
	__HOOK_IA32(__NR32_##name, real_sys32_##name, hooked_sys32_##name);

#define __UNHOOK_IA32(F, RF)								\
	DEBUG(KERN_INFO "UNHOOK_IA32 " #F "\n");				\
	ia32_sys_call_table[F] = (void *)RF;

#define UNHOOK_IA32(name)									\
	__UNHOOK_IA32(__NR32_##name, real_sys32_##name)



#define __INCR(F)	\
	atomic_inc(&__counter_info___NR_##F.counter);

#define __DECR(F)	\
	atomic_dec(&__counter_info___NR_##F.counter);

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