#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include <linux/kernel.h>
#include <linux/module.h>

#include "msgs.h"
#include "utils.h"
#include "sct_hook.h"

int get_client_pid(void);

int register_procmon_sysctl(void);
void unregister_procmon_sysctl(void);

#endif
