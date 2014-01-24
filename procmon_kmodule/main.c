#include "main.h"

/*****************************************************************************\
| Main methods: _init and _exit                                               |
\*****************************************************************************/

int monks_state = 0, client_pid = -1;

static int __init hook_init(void){
	static int netlink_id;

	monks_info("================\n");
	monks_info("Starting monks\n");

	netlink_id = nl_init();
	if(netlink_id == -2){
		monks_error("Can't create netlink thread\n");
	}else if(netlink_id == -1){
		monks_info("Error creating socket.\n");
	}else{
		monks_info("Acquired NETLINK socket (%d)\n", netlink_id);
	}

	set_monks_control_netlink_id(&netlink_id);

	hook_calls();

	if(!register_monks_controls()){
		monks_error("Error creating fs control interface in /proc/sys/monks/\n");
		return -ENOMEM;
	}

	monks_info("Successfully loaded\n");

	return 0;
}

static void __exit hook_exit(void){
	monks_state = 0;

	monks_info("Stopping monks\n");

	unregister_monks_controls();

	unhook_calls();
	
	nl_halt();

	monks_info("Successfully unloaded\n");
	monks_info("================\n");
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Monks alternative for Linux");
