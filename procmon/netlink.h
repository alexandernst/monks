#ifndef NETLINK_H
#define NETLINK_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>

#include <sys/socket.h>
#include <linux/netlink.h>

#include "../common/mem_ops.h"

#define MAX_PAYLOAD 1024

int get_netlink_id(void);
int net_init(void);

#endif