#include "control.h"

/*****************************************************************************\
| /proc/sys state and methods related to the control of procmon               |
\*****************************************************************************/

static int nsyscalls_state_table = 0;
static ctl_table *syscalls_state_table;
static struct ctl_table_header *procmon_table_header;

static ctl_table state_table[] = {
	{
		.procname = "syscalls", .mode = 0555
	},
	{
		.procname = "state", .mode = 0666,
		.proc_handler = &proc_dointvec_minmax,
		.data = &procmon_state, .maxlen	= sizeof(int),
		.extra1 = "\x00\x00\x00\x00" /*0*/, .extra2 = "\x01\x00\x00\x00" /*1*/
	},
	{
		.procname = "netlink", .mode = 0444,
		.proc_handler = &proc_dointvec,
		.data = "\xFF\xFF\xFF\xFF" /*-1, assign after nl_init()*/, .maxlen = sizeof(int)
	},
	{
		.procname = "client_pid", .mode = 0666,
		.proc_handler = &proc_dointvec,
		.data = &client_pid, .maxlen = sizeof(pid_t)
	},
	{ 0 }
};

static ctl_table procmon_table[] = {
	{
		.procname = "procmon", .mode = 0555,
		.child = state_table
	},
	{ 0 }
};

int register_procmon_controls(void){

	syscalls_state_table = kcalloc(nsyscalls_state_table, sizeof(struct ctl_table), GFP_KERNEL);
	if(!syscalls_state_table){
		return 0;
	}

	procmon_table_header = register_sysctl_table(procmon_table);
	if(!procmon_table_header){
		return 0;
	}else{
		return 1;
	}
}

void add_syscalls_state_table_entry(char *procname, int *state){
	if(nsyscalls_state_table == 0){
		nsyscalls_state_table = 2;
		syscalls_state_table = kcalloc(nsyscalls_state_table, sizeof(struct ctl_table), GFP_KERNEL);
	}else{
		nsyscalls_state_table++;
		syscalls_state_table = krealloc(syscalls_state_table, nsyscalls_state_table * sizeof(struct ctl_table), GFP_KERNEL);
		memset(&(syscalls_state_table[nsyscalls_state_table - 1]), 0, sizeof(struct ctl_table));
	}

	syscalls_state_table[nsyscalls_state_table - 2].procname = procname;
	syscalls_state_table[nsyscalls_state_table - 2].mode = 0666;
	syscalls_state_table[nsyscalls_state_table - 2].proc_handler = &proc_dointvec_minmax;
	syscalls_state_table[nsyscalls_state_table - 2].data = state;
	syscalls_state_table[nsyscalls_state_table - 2].maxlen	= sizeof(int);
	syscalls_state_table[nsyscalls_state_table - 2].extra1 = "\x00\x00\x00\x00"; /*0*/
	syscalls_state_table[nsyscalls_state_table - 2].extra2 = "\x01\x00\x00\x00"; /*1*/

	state_table[0].child = syscalls_state_table;
}

void set_procmon_control_netlink_id(int *netlink_id){
	procmon_table[0].child[2].data = netlink_id;
}

void unregister_procmon_controls(void){
	unregister_sysctl_table(procmon_table_header);
	kfree(syscalls_state_table);
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/