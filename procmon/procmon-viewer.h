#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>

#include "../common/mem_ops.h"
#include "../common/structures.h"
#include "../common/deserialize.h"

#ifndef __NO_KMOD__
#include "lkm.h"
#endif

#include "ui.h"
#include "netlink.h"

#define PROCMON_VERSION 0.1
#define PROCMON_MODULE_NAME "procmon"
#define PROCMON_MODULE_PATH "./procmon.ko"

#define MAXEVENTS 2
#define MEM_LIMIT 30000

void do_segfault();
int read_from_kb(void);
void read_from_socket(int sock_fd, struct nlmsghdr *nlh);
void add_data(syscall_info *i);
void free_info(syscall_info *i);