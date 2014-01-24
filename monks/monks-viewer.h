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
#include "utils.h"
#include "netlink.h"

#define MONKS_VERSION 0.1
#define MONKS_MODULE_NAME "monks"
#define MONKS_MODULE_PATH "./monks.ko"

#define MAXEVENTS 2
#define MEM_LIMIT 30000

void do_segfault();
void add_data(syscall_intercept_info *i);
void free_data(syscall_intercept_info *i);