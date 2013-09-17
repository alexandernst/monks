#include "syshijack.h"

/*****************************************************************************\
| Main methods: _init and _exit                                               |
\*****************************************************************************/

static int __init hook_init(void){
	if(register_procmon_sysctl() != 0){
		DEBUG(KERN_INFO "Error creating /proc/sys/procmon/state\n");
		return -ENOMEM;
	}

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

	unregister_procmon_sysctl();
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Procmon alternative for Linux");