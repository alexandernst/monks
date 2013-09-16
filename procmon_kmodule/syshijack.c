#include "syshijack.h"

/*****************************************************************************\
| Main methods: _init and _exit                                               |
\*****************************************************************************/

static int __init hook_init(void){
	proc_write_entry = proc_create("procmon", 0666, NULL, &proc_file_fops);
	if(!proc_write_entry){
		DEBUG(KERN_INFO "Error creating proc entry\n");
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

	remove_proc_entry("procmon", NULL);
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Procmon alternative for Linux");