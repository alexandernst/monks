#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

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
#include <linux/sysctl.h>
#include <linux/fdtable.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#ifdef CONFIG_IA32_EMULATION
#include "unistd_32.h"
#endif

#include "utils.h"
#include "sct_hook.h"
#include "syscalls/everything.h"

/*****************************************************************************\
| Define if debugging is enabled or disabled                                  |
\*****************************************************************************/

#define debug 1

#if debug == 1
#define DEBUG(...) printk(__VA_ARGS__);
#else
#define DEBUG(...)
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

void activate(void);
void deactivate(void);
int is_active(void);

int register_procmon_sysctl(void);
void unregister_procmon_sysctl(void);

#endif