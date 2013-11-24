#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#include <linux/kernel.h>
#include <linux/module.h>

extern int procmon_state, client_pid;

int register_procmon_controls(void);
void set_procmon_control_netlink_id(int *netlink_id);
void unregister_procmon_controls(void);

#endif