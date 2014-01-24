#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#include <linux/slab.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/module.h>

extern int monks_state, client_pid;

int register_monks_controls(void);
void add_syscalls_state_table_entry(char *procname, int *state);
void set_monks_control_netlink_id(int *netlink_id);
void unregister_monks_controls(void);

#endif