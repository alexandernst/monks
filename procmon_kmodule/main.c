#include "main.h"

/*****************************************************************************\
| /proc/sys state and methods related to the control of procmon               |
\*****************************************************************************/

int procmon_state = 0, client_pid = -1;
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

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

/*****************************************************************************\
| Main methods: _init and _exit                                               |
\*****************************************************************************/

static int __init hook_init(void){
	static int netlink_id;

	procmon_info("================\n");
	procmon_info("Starting procmon\n");

	netlink_id = nl_init();
	if(netlink_id == -2){
		procmon_error("Can't create netlink thread\n");
	}else if(netlink_id == -1){
		procmon_info("Error creating socket.\n");
	}else{
		procmon_info("Acquired NETLINK socket (%d)\n", netlink_id);
	}

	procmon_table[0].child[1].data = &netlink_id;

	procmon_table_header = register_sysctl_table(procmon_table);
	if(!procmon_table_header){
		procmon_error("Error creating fs control interface in /proc/sys/procmon/\n");
		return -ENOMEM;
	}

	hook_calls();

	procmon_info("Successfully loaded\n");

	return 0;
}

static void __exit hook_exit(void){
	procmon_state = 0;

	procmon_info("Stopping procmon\n");

	unhook_calls();

	while(!safe_to_unload()){
		msleep_interruptible(500);
	}

	unregister_sysctl_table(procmon_table_header);
	
	nl_halt();

	procmon_info("Successfully unloaded\n");
	procmon_info("================\n");
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Procmon alternative for Linux");
