#include <linux/mm.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fdtable.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#include <asm/unistd.h>
#ifdef CONFIG_IA32_EMULATION
#include "unistd_32.h"
#endif

/*****************************************************************************\
| HOOK MACROS                                                                 |
| F, RF and FF stand for:                                                     |
| F = FUNCTION as defined in include/linux/syscalls.h                         |
| RF = REAL FUNCTION as in the function in which we will save F               |
| FF = FAKE FUNCTION as in the function which we'll be using to fake F        |
\*****************************************************************************/

#define HOOK(F, RF, FF) RF = sys_call_table[F]; sys_call_table[F] = FF;
#ifdef CONFIG_IA32_EMULATION
	#define HOOK_IA32(F, RF, FF) RF = ia32_sys_call_table[F]; ia32_sys_call_table[F] = FF;
#endif

#define UNHOOK(F, RF) sys_call_table[F] = RF;
#ifdef CONFIG_IA32_EMULATION
	#define UNHOOK_IA32(F, RF) ia32_sys_call_table[F] = RF;
#endif

/*****************************************************************************\
|                                      END                                    |
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

typedef struct syscall_intercept_info{
	char *pname;
	pid_t pid;
	char *operation;
	char *path;
	unsigned int result;
	char *details;
} syscall_info;

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

int make_rw(unsigned long address);
int make_ro(unsigned long address);

unsigned long clear_and_return_cr0(void);
void setback_cr0(unsigned long val);

int get_sct(void);
int set_sct_rw(void);
int set_sct_ro(void);

void hook_calls(void);
void unhook_calls(void);

/*****************************************************************************\
| Utils                                                                       |
\*****************************************************************************/

void print_info(syscall_info *i);
char *path_from_fd(unsigned int fd);

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/