#ifndef NETLINK_H_INCLUDED
#define NETLINK_H_INCLUDED

#include <net/sock.h>
#include <linux/kthread.h>
#include <linux/version.h>

#include "msgs.h"
#include "../common/mem_ops.h"
#include "../common/serialize.h"

extern int procmon_state, client_pid;

int nl_init(void);
void nl_halt(void);
void nl_send(syscall_info *i);

#endif