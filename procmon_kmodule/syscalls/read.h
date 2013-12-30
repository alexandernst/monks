#ifndef READ_H_INCLUDED
#define READ_H_INCLUDED

#include "../utils.h"
#include "../netlink.h"
#include "../sct_hook.h"
#include "../../common/mem_ops.h"

extern asmlinkage void hooked_sys_read(unsigned int fd, char __user *buf, size_t count);
#ifdef CONFIG_IA32_EMULATION
extern asmlinkage void hooked_sys32_read(unsigned int fd, char __user *buf, size_t count);
#endif

#endif