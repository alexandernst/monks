#ifndef HOOKFNS_H_INCLUDED
#define HOOKFNS_H_INCLUDED

#include "syshijack.h"

//Warnings because of lazy GCC devs
//http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53119
struct counter_info {
	atomic_t counter;
	char *name;
};

extern struct counter_info __start_counters;
extern struct counter_info __stop_counters;

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
															\
	static struct counter_info __counter_info_##F			\
	__attribute((__section__("counters")))					\
	__attribute((__used__)) = {								\
		.name = #F,											\
	};														\
	atomic_set(&__counter_info_##F.counter, 0);				\
															\
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
															\
	static struct counter_info __counter_info_##F			\
	__attribute((__section__("counters")))					\
	__attribute((__used__)) = {								\
		.name = #F"_32",									\
	};														\
	atomic_set(&__counter_info_##F.counter, 0);				\
															\
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



#define __INCR(F)											\
do{															\
	struct counter_info *iter = &__start_counters;			\
	for(; iter < &__stop_counters; ++iter){					\
		if(strcmp("__NR_" #F, iter->name) == 0){			\
			atomic_inc(&iter->counter);						\
		}													\
	}														\
}while(0)

#define __DECR(F)											\
do{															\
	struct counter_info *iter = &__start_counters;			\
	for(; iter < &__stop_counters; ++iter){					\
		if(strcmp("__NR_" #F, iter->name) == 0){			\
			atomic_dec(&iter->counter);						\
		}													\
	}														\
}while(0)

#define __INCR32(F)											\
do{															\
	struct counter_info *iter = &__start_counters;			\
	for(; iter < &__stop_counters; ++iter){					\
		if(strcmp("__NR_" #F "_32", iter->name) == 0){		\
			atomic_inc(&iter->counter);						\
		}													\
	}														\
}while(0)

#define __DECR32(F)											\
do{															\
	struct counter_info *iter = &__start_counters;			\
	for(; iter < &__stop_counters; ++iter){					\
		if(strcmp("__NR_" #F "_32", iter->name) == 0){		\
			atomic_dec(&iter->counter);						\
		}													\
	}														\
}while(0)

#endif /* CONFIG_IA32_EMULATION */

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void hook_calls(void);
void unhook_calls(void);

#endif