#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <linux/kernel.h>
#include <linux/module.h>

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

#include "msgs.h"
#include "sct_hook.h"

void activate(void);
void deactivate(void);
int is_active(void);
int get_client_pid(void);

int register_procmon_sysctl(void);
void unregister_procmon_sysctl(void);

#endif
