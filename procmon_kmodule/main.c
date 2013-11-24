#include "main.h"

/*****************************************************************************\
| Main methods: _init and _exit                                               |
\*****************************************************************************/

int procmon_state = 0, client_pid = -1;

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

	set_procmon_control_netlink_id(&netlink_id);

	if(!register_procmon_controls()){
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

	unregister_procmon_controls();
	
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
