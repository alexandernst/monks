#ifndef HOOKFNS_H_INCLUDED
#define HOOKFNS_H_INCLUDED

#include "syshijack.h"

typedef struct syscall_info {
	atomic_t counter;
	char *name;
	int is32;
	int __NR_;
	void *ff;
	void *rf;
} __attribute__((packed)) syscall_info_t;

extern syscall_info_t __start_syscalls[];
extern syscall_info_t __stop_syscalls[];

/*****************************************************************************************\
| HOOK MACROS                                                                             |
| __NR_#F, real_sys_##F and hooked_sys_##F stand for:                                     |
| __NR_#F = FUNCTION as defined in include/linux/syscalls.h                               |
| real_sys_##F = REAL FUNCTION as in the function in which we will save __NR_##F          |
| hooked_sys_##F = FAKE FUNCTION as in the function which we'll be using to fake __NR_##F |
\*****************************************************************************************/

#define __REGISTER_SYSCALL(F)									\
	static syscall_info_t __syscall_info___NR_##F				\
	__attribute__((section(".syscalls"), aligned(1))) = {		\
		.counter = ATOMIC_INIT(0),								\
		.name = "__NR_" #F,										\
		.is32 = 0,												\
		.__NR_ = __NR_##F,										\
		.ff = hooked_sys_##F,									\
		.rf = &real_sys_##F,									\
	};

#define __INCR(F)												\
	atomic_inc(&__syscall_info___NR_##F.counter);

#define __DECR(F)												\
	atomic_dec(&__syscall_info___NR_##F.counter);

#define __SYSCALL(F)											\
	((typeof(real_sys_##F))__syscall_info___NR_##F.rf)

#ifdef CONFIG_IA32_EMULATION

#define __REGISTER_SYSCALL32(F)									\
	static syscall_info_t __syscall_info___NR32_##F				\
	__attribute__((section(".syscalls"), aligned(1))) = {		\
		.counter = ATOMIC_INIT(0),								\
		.name = "__NR32_" #F,									\
		.is32 = 1,												\
		.__NR_ = __NR32_##F,									\
		.ff = hooked_sys32_##F,									\
		.rf = &real_sys32_##F,									\
	};

#define __INCR32(F)												\
	atomic_inc(&__syscall_info___NR32_##F.counter);

#define __DECR32(F)												\
	atomic_dec(&__syscall_info___NR32_##F.counter);

#define __SYSCALL32(F)											\
	((typeof(real_sys32_##F))__syscall_info___NR32_##F.rf)

#endif /* CONFIG_IA32_EMULATION */

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void setback_cr0(unsigned long val);
unsigned long clear_and_return_cr0(void);

int get_sct(void);
int set_sct_rw(void);
int set_sct_ro(void);

void hook_calls(void);
void unhook_calls(void);
int safe_to_unload(void);

#endif