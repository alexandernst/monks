#include "control.h"

/*****************************************************************************\
| /proc/sys state and methods related to the control of procmon               |
\*****************************************************************************/

static struct ctl_table_header *procmon_table_header;

static ctl_table state_table[] = {
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
	procmon_table_header = register_sysctl_table(procmon_table);
	if(!procmon_table_header){
		return 0;
	}else{
		return 1;
	}
}

void set_procmon_control_netlink_id(int *netlink_id){
	procmon_table[0].child[1].data = netlink_id;
}

void unregister_procmon_controls(void){
	unregister_sysctl_table(procmon_table_header);
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/