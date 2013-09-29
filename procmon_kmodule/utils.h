#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fdtable.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>

#include <net/sock.h>

#include "../common/mem_ops.h"
#include "../common/serialize.h"
#include "../common/structures.h"

/*****************************************************************************\
| Utils                                                                       |
\*****************************************************************************/

#define debug 1

#if debug == 1
#define DEBUG(...) printk(__VA_ARGS__);
#else
#define DEBUG(...)
#endif

void nl_init(void);
void nl_halt(void);
void nl_recv(struct sk_buff *skb);
void nl_send(syscall_info *i);

char *path_from_fd(unsigned int fd);

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif