#include "syshijack.h"
 
/*****************************************************************************\
| /proc vars and methods related to the control of procmon                    |
\*****************************************************************************/

char proc_data[1];
struct proc_dir_entry *proc_write_entry;

ssize_t read_proc(struct file *file, char __user *buf, size_t count, loff_t *pos){
	int ret;
	if(*pos == 0){
		memcpy(buf, proc_data, 1);
		*pos = (loff_t)1;
		ret = 1;
	}else{
		ret = 0;
	}
	return ret;
}

ssize_t write_proc(struct file *file, const char __user *buf, size_t count, loff_t *pos){
	if(count > 2)
		return -EINVAL;

	if(copy_from_user(proc_data, buf, 1))
		return -EFAULT;

	if(strcmp("1", proc_data) == 0){
		hook_calls();
	}else if(strcmp("0", proc_data) == 0){
		unhook_calls();
	}
	
	return count;
}

const struct file_operations proc_file_fops = {
	.owner = THIS_MODULE,
	.read  = read_proc,
	.write = write_proc,
};

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/