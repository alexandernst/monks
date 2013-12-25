#ifndef SCT_HOOK_H_INCLUDED
#define SCT_HOOK_H_INCLUDED

/*****************************************************************************\
| In case we are on IA32 system we need to get the identifier for both x64    |
| and x86 archs. And we can't do that only with #include <asm/unistd.h>       |
| because this way we'll only get the identifiers of one of the archs, the    |
| one that we're currently compiling on.                                      |
| Also, even if we could trick somehow the compiler to include both           |
| identifiers, there is another problem: we will get different values with    |
| the same name.                                                              |
| Example: __NR_read and __NR_read, both have the same name, but on           |
| x86 it's value is N, while on x64 it's value is M. That's why we need to do |
| some magic.                                                                 |
| When we're on x64 and ia32 emulation is enabled, we'll search for the       |
| <asm/unistd_32.h> file, replace all "__NR_xxx" with "__NR32_xxx" and then   |
| use them for the ia32 syscall table.                                        |
\*****************************************************************************/

#include <asm/unistd.h>
#ifdef CONFIG_IA32_EMULATION
#include "unistd_32.h"
#endif

#include <linux/preempt.h>
#include <linux/stop_machine.h>

#include "msgs.h"
#include "utils.h"
#include "control.h"
#include "../udis86/udis86.h"
#include "../common/mem_ops.h"

#define to_x86_ptr(x) (void *)(x)
#define to_x64_ptr(x) (void *)(0xffffffff00000000 | x)

typedef struct syscall_info_t {
	atomic_t *counter;
	char *name;
	int is32;
	int state;
	int __NR_;
	void *ff;
	void *rf;
} __attribute__((packed)) syscall_info_t;

extern syscall_info_t __start_syscalls[];
extern syscall_info_t __stop_syscalls[];

#ifdef __i386__
struct idt_descriptor{
	unsigned short offset_low;
	unsigned short selector;
	unsigned char zero;
	unsigned char type_flags;
	unsigned short offset_high;
} __attribute__((packed));
#elif defined(CONFIG_IA32_EMULATION)
struct idt_descriptor{
	unsigned short offset_low;
	unsigned short selector;
	unsigned char zero1;
	unsigned char type_flags;
	unsigned short offset_middle;
	unsigned int offset_high;
	unsigned int zero2;
} __attribute__((packed));
#endif

struct idtr{
	unsigned short limit;
	void *base;
} __attribute__((packed));

/*****************************************************************************************\
| REGISTER MACROS                                                                         |
| __NR_#F, real_sys_##F and hooked_sys_##F stand for:                                     |
| __NR_#F = FUNCTION as defined in include/linux/syscalls.h                               |
| real_sys_##F = REAL FUNCTION as in the function in which we will save __NR_##F          |
| hooked_sys_##F = FAKE FUNCTION as in the function which we'll be using to fake __NR_##F |
|                                                                                         |
| NOTE: The __REGISTER_SYSCALL/32 macros are called at compile time and the values are    |
| stored in a ELF section called 'syscalls'.                                              |
| That section is iterated at run time, once when hooking and once when unhooking. That   |
| is how we know the details about every single syscall at run time, and that is how we   |
| get the necessary information to patch the syscall table.                               | 
\*****************************************************************************************/

#define __REGISTER_SYSCALL(F)									\
	static syscall_info_t __syscall_info___NR_##F				\
	__attribute__((section(".syscalls"), aligned(1), used)) = { \
		.counter = NULL,										\
		.name = "__NR_" #F,										\
		.is32 = 0,												\
		.state = 1,												\
		.__NR_ = __NR_##F,										\
		.ff = hooked_sys_##F,									\
		.rf = &real_sys_##F,									\
	};

//8 bytes for return address + 8 bytes for stach push
//TODO: Write better docs about this giant hack
//tips for when about to write: stack
//also, size of value or address
#define __GET_SYSCALL_RESULT(x)									\
	__asm__ __volatile__(										\
		".intel_syntax noprefix;"								\
		"mov %0, [rbp + 8 + 8];"								\
		".att_syntax;"											\
		: "=r" (x)												\
		:														\
		:														\
	);

#ifdef CONFIG_IA32_EMULATION

#define __REGISTER_SYSCALL32(F)									\
	static syscall_info_t __syscall_info___NR32_##F				\
	__attribute__((section(".syscalls"), aligned(1), used)) = { \
		.counter = NULL,										\
		.name = "__NR32_" #F,									\
		.is32 = 1,												\
		.state = 1,												\
		.__NR_ = __NR32_##F,									\
		.ff = hooked_sys32_##F,									\
		.rf = &real_sys32_##F,									\
	};

#define __INCR32(F)												\
	atomic_inc(__syscall_info___NR32_##F.counter);

#define __DECR32(F)												\
	atomic_dec(__syscall_info___NR32_##F.counter);

#define __STATE32(F)											\
	__syscall_info___NR32_##F.state

#define __REAL_SYSCALL32(F)										\
	((typeof(real_sys32_##F))__syscall_info___NR32_##F.rf)

#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

unsigned int ud_find_insn_arg(void *entry, int limit, enum ud_mnemonic_code insn_mne, int insn_len);

void *get_writable_sct(void *sct_addr);
#if defined(__i386__) || defined(CONFIG_IA32_EMULATION)
#ifdef __i386__
void *get_sys_call_table(void);
#elif defined(__x86_64__)
void *get_ia32_sys_call_table(void);
#endif
#endif

#ifdef __x86_64__
void *get_sys_call_table(void);
#endif

void hook_calls(void);
void unhook_calls(void);
int safe_to_unload(void);

#endif
