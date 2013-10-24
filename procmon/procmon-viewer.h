#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

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

#define MEM_LIMIT 3000

void do_segfault();
void add_data(syscall_info *i);
void free_info(syscall_info *i);