#include "main.h"

/*****************************************************************************\
| /proc/sys state and methods related to the control of procmon               |
\*****************************************************************************/

int state = 0, min = 0, max = 1, client_pid = -1;
static struct ctl_table_header *procmon_table_header;

static ctl_table state_table[] = {
	{
		.procname = "state", .mode = 0666,
		.proc_handler = &proc_dointvec_minmax,
		.data = &state, .maxlen	= sizeof(int),
		.extra1 = &min, .extra2 = &max
	},
	{
		.procname = "netlink", .mode = 0444,
		.proc_handler = &proc_dointvec,
		.data = &nl_id, .maxlen = sizeof(int)
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

void activate(void){ state = 1; }
void deactivate(void){ state = 0; }
int is_active(void){ return state; }
int get_client_pid(void){ return client_pid; }

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

/*****************************************************************************\
| Main methods: _init and _exit                                               |
\*****************************************************************************/

static int __init hook_init(void){
	procmon_table_header = register_sysctl_table(procmon_table);
	if(!procmon_table_header){
		DEBUG(KERN_INFO "Error creating /proc/sys/procmon/state\n");
		return -ENOMEM;
	}

	nl_init();

	hook_calls();

	return 0;
}

static void __exit hook_exit(void){
	if(is_active()){ //just in case user did not deactivate
		deactivate();
	}
	unhook_calls();

	while(!safe_to_unload()){
		msleep_interruptible(500);
	}

	unregister_sysctl_table(procmon_table_header);
	
	nl_halt();
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Procmon alternative for Linux");