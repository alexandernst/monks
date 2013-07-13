#include "syshijack.h"

void print_info(syscall_info *i){
	printk(KERN_INFO
		"%s"      /* Process name */
		"%15u"    /* PID          */
		"%10s"    /* Operation    */
		"%15s"    /* Path         */
		"%50u"    /* Result       */
		"%20s\n", /* Details      */

		i->pname,
		i->pid,
		i->operation,
		i->path,
		i->result,
		i->details
	);
}

char *path_from_fd(unsigned int fd){
	char *tmp;
	char *pathname = "";
	char *rpathname = kmalloc(1, GFP_KERNEL);

	struct file *file;
	struct path path;
	struct files_struct *files = current->files;

	spin_lock(&files->file_lock);
	file = fcheck_files(files, fd);
	if (!file) {
		spin_unlock(&files->file_lock);
		return rpathname;
	}

	path = file->f_path;
	path_get(&file->f_path);
	spin_unlock(&files->file_lock);

	
	tmp = (char *)__get_free_page(GFP_TEMPORARY);
	if(!tmp){
		path_put(&path);
		return rpathname;
	}

	pathname = d_path(&path, tmp, PAGE_SIZE);
	path_put(&path);

	if(IS_ERR(pathname)){
		free_page((unsigned long)tmp);
		return rpathname;
	}

	rpathname = kstrdup(pathname, GFP_KERNEL);
	
	free_page((unsigned long)tmp);
	
	return rpathname;
}