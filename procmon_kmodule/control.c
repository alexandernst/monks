#include "syshijack.h"
 
/*****************************************************************************\
| /proc vars and methods related to the control of procmon                    |
\*****************************************************************************/

char proc_data[1] = "0";
struct proc_dir_entry *proc_write_entry;

void activate(void){
	memcpy(proc_data, "1", 1);
}

void deactivate(void){
	memcpy(proc_data, "0", 1);
}

int is_active(void){
	return strcmp("1", proc_data) == 0;
}

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
		activate();
	}else if(strcmp("0", proc_data) == 0){
		deactivate();
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