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
#include "udis_utils.h"
#include "../common/mem_ops.h"

typedef struct syscall_info_t {
	char *name;
	int is32;
	int state;
	int __NR_;
	void *pre;
	void *post;
	void *rf;
} __attribute__((packed)) syscall_info_t;

extern syscall_info_t __start_syscalls[];
extern syscall_info_t __stop_syscalls[];

#define for_each_syscall(item)		\
	for (item = __start_syscalls; item < __stop_syscalls; item++)

#ifdef CONFIG_X86_32
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

/*****************************************************************************\
| REGISTER MACROS                                                             |
| __NR_#F, real_sys_##F and hooked_sys_##F stand for:                         |
|                                                                             |
| __NR_#F = FUNCTION as defined in include/linux/syscalls.h                   |
|                                                                             |
| real_sys_##F = REAL FUNCTION, means the function in which we will save      |
| __NR_##F                                                                    |
|                                                                             |
| hooked_sys_##F = FAKE FUNCTION, means the function which we'll be using to  |
| fake __NR_##F                                                               |
|                                                                             |
| NOTE: The __REGISTER_SYSCALL/32 macros are called at compile time and the   |
| values are stored in a ELF section called 'syscalls'.                       |
| That section is iterated at run time, once when hooking and once when       |
| unhooking. That is how we know the details about every single syscall at    |
| run time, and that is how we get the necessary information to patch the     |
| syscall table.                                                              |
\*****************************************************************************/

#define __REGISTER_SYSCALL(F)                                                 \
	static syscall_info_t __syscall_info___NR_##F                             \
	__attribute__((section(".syscalls"), aligned(1), used)) = {               \
		.name = "__NR_" #F,                                                   \
		.is32 = 0,                                                            \
		.state = 1,                                                           \
		.__NR_ = __NR_##F,                                                    \
		.pre = hooked_sys_pre_##F,                                            \
		.post = hooked_sys_post_##F,                                          \
	};

#ifdef CONFIG_IA32_EMULATION
#define __REGISTER_SYSCALL32(F)                                               \
	static syscall_info_t __syscall_info___NR32_##F                           \
	__attribute__((section(".syscalls"), aligned(1), used)) = {               \
		.name = "__NR32_" #F,                                                 \
		.is32 = 1,                                                            \
		.state = 1,                                                           \
		.__NR_ = __NR32_##F,                                                  \
		.pre = hooked_sys32_pre_##F,                                          \
		.post = hooked_sys32_post_##F,                                        \
	};
#endif

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| RESULT MACROS                                                               |
| Each fake syscall function should have access to the result of the real     |
| syscall call. This macro gives access to that result value by accessing the |
| stack of the caller. Each fake syscall is called from it's stub. Each stub  |
| reserves 28 bytes on the stack. 4 bytes for the syscall result and 24 bytes |
| for the 6 arguments that a syscall can have. The first 4 bytes are the ones |
| that this macro accesses. We know (guaranteed because of how the stub works)|
| that there isn't anything else on the stack, so we can play with it without |
| any real danger. To access those first 4 bytes we need to keep in mind a    |
| few things. There are 4 bytes on the stack for the return address. Also,    |
| when the fake syscall is called, the EBP value is pushed on the stack,      |
| which means 4 more bytes. That means that if we get the current EBP and add |
| it 8 + 8 we'll get directly into the caller's stack, which is what we want. |
| From there on, we only need to add yet another 6 * 4 bytes and we'll reach  |
| the syscall return value. Please have a look at stubs.S for an actual       |
| graphic representation of how the stack looks like when a stub is being     |
| executed.                                                                   |
|                                                                             |
| NOTE: Don't confuse the way the stack works. We need to ADD 8 bytes         |
| instead of REST 8 bytes because the stacks grows downwards. Also note that  |
| this explanation is valid for x86 platforms. On x64 we'll have RAX instead  |
| of EAX, RBP instead of EBP, the return value will take 8 bytes and a push   |
| will take another 8 bytes, so that will make 16 bytes in total.             |
|                                                                             |
| NOTE: Keep in mind that we are reading 4 bytes (8 bytes on x64) from the    |
| stack, which can contain a value or an address. It's up to you to know what |
| the real syscall will return. If the returned value fits in 4 bytes, you    |
| just pass a variable to this macro. If the syscall returns a pointer, then  |
| you'll need to pass this macro a pointer.                                   |
\*****************************************************************************/

#ifdef CONFIG_X86_32
#define __GET_SYSCALL_RESULT(x)                                               \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov %0, [ebp + 4 + 4 + 24];"                                         \
		".att_syntax;"                                                        \
		: "=r" (x)                                                            \
	);
#else
#define __GET_SYSCALL_RESULT(x)                                               \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov %0, [rbp + 8 + 8 + 48];"                                         \
		".att_syntax;"                                                        \
		: "=r" (x)                                                            \
	);
#endif

#ifdef CONFIG_IA32_EMULATION
#define __GET_SYSCALL_RESULT32(x)                                             \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov %0, [rbp + 8 + 8 + 24];"                                         \
		".att_syntax;"                                                        \
		: "=r" (x)                                                            \
	);
#endif

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| HOOK INFO MACROS                                                            |
| The same way we use the stack to store the result of a syscall, we use it   |
| to store some information about the syscall that is being ran. Each stub    |
| calls two functions (plus the real syscall): the pre-hook and the post-hook.|
| When the pre-hook is called, we save the arguments and create a basic node  |
| of syscall_intercept_info type which is filled with some data. Then it's    |
| saved here, on the stack, so we can recover it when we're in the post-hook. |
\*****************************************************************************/

#ifdef CONFIG_X86_32
#define __SET_SYSCALL_HOOK_INFO(x)                                            \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov [ebp + 4 + 4 + 28], %0;"                                         \
		".att_syntax;"                                                        \
		:                                                                     \
		: "r" (x)                                                             \
	);
#else
#define __SET_SYSCALL_HOOK_INFO(x)                                            \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov [rbp + 8 + 8 + 56], %0;"                                         \
		".att_syntax;"                                                        \
		:                                                                     \
		: "r" (x)                                                             \
	);
#endif

#ifdef CONFIG_IA32_EMULATION
#define __SET_SYSCALL_HOOK_INFO32(x)                                          \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov [rbp + 8 + 8 + 28], %0;"                                         \
		".att_syntax;"                                                        \
		:                                                                     \
		: "r" (x)                                                             \
	);
#endif

#ifdef CONFIG_X86_32
#define __GET_SYSCALL_HOOK_INFO(x)                                            \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov %0, [ebp + 4 + 4 + 28];"                                         \
		".att_syntax;"                                                        \
		: "=r" (x)                                                            \
	);
#else
#define __GET_SYSCALL_HOOK_INFO(x)                                            \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov %0, [rbp + 8 + 8 + 56];"                                         \
		".att_syntax;"                                                        \
		: "=r" (x)                                                            \
	);
#endif

#ifdef CONFIG_IA32_EMULATION
#define __GET_SYSCALL_HOOK_INFO32(x)                                          \
	__asm__ __volatile__(                                                     \
		".intel_syntax noprefix;"                                             \
		"mov %0, [rbp + 8 + 8 + 28];"                                         \
		".att_syntax;"                                                        \
		: "=r" (x)                                                            \
	);
#endif

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

void *get_writable_sct(void *sct_addr);
#if defined(CONFIG_X86_32) || defined(CONFIG_IA32_EMULATION)
#ifdef CONFIG_X86_32
void *get_sys_call_table(void);
#elif defined(CONFIG_X86_64)
void *get_ia32_sys_call_table(void);
#endif
#endif

#ifdef CONFIG_X86_64
void *get_sys_call_table(void);
#endif

void hook_calls(void);
void unhook_calls(void);

#endif
