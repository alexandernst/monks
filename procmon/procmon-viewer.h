#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "../common/mem_ops.h"
#include "../common/structures.h"
#include "../common/deserialize.h"

#ifndef __NO_KMOD__
#include "lkm.h"
#endif

#define PROCMON_VERSION 0.1
#define PROCMON_MODULE_NAME "procmon"
#define PROCMON_MODULE_PATH "./procmon.ko"

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024

int net_init();
void print_info(syscall_info *i);