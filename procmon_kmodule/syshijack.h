#ifndef SYSHIJACK_H_INCLUDED
#define SYSHIJACK_H_INCLUDED

#include <asm/page.h>
#include <asm/unistd.h>
#include <asm/uaccess.h>

#include <linux/mm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fdtable.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>

#ifdef CONFIG_IA32_EMULATION
#include "unistd_32.h"
#endif

#include "utils.h"
#include "control.h"
#include "fns.h"
#include "syscalls_hash.h"
#include "control_ro_rw_syscall_table.h"

/*****************************************************************************\
| Define which method (1, 2 or 3) will be used to set sct to RO/RW            |
| Method 1 will use kernel pages and vmap                                     |
| Method 2 will use virtual address                                           |
| Method 3 will disable cr0, reg 16                                           |
| Algo define if debugging is enabled or disabled                             |
\*****************************************************************************/

#define method 3
#define debug 1

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/


/*****************************************************************************\
| Define debug macro                                                          |
\*****************************************************************************/

#if debug == 1
#define DEBUG(...) printk(__VA_ARGS__);
#else
#define DEBUG(...)
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/


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
sys_call_table[F] = FF;                       \
REGISTER(F);

#ifdef CONFIG_IA32_EMULATION
#define HOOK_IA32(F, RF, FF)                  \
DEBUG(KERN_INFO "HOOKING_IA32 " #F "\n");     \
RF = (void *)ia32_sys_call_table[F];          \
ia32_sys_call_table[F] = FF;                  \
REGISTER(F);
#endif

#define UNHOOK(F, RF)                         \
DEBUG(KERN_INFO "UNHOOKING " #F "\n");        \
sys_call_table[F] = RF;

#ifdef CONFIG_IA32_EMULATION
#define UNHOOK_IA32(F, RF)                    \
DEBUG(KERN_INFO "UNHOOKING_IA32 " #F "\n");   \
ia32_sys_call_table[F] = RF;
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/


/*****************************************************************************\
| Main Procmon functions                                                      |
\*****************************************************************************/

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

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif