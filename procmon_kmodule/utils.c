#include "utils.h"

void print_info(syscall_info *i){
	printk(KERN_INFO
		"%-15.10s"       /* Process name */
		"%10u"           /* PID          */
		" %-15.10s"      /* Operation    */
		"%-50.45s"       /* Path         */
		"%-10.10s"       /* Result       */
		" %-200.200s\n", /* Details      */

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

int vasprintf(char **ret, const char *format, va_list args){
	int count;
	char *buffer;
	va_list copy;
	va_copy(copy, args);

	*ret = 0;

	count = vsnprintf(NULL, 0, format, args);
	if (count >= 0){
		buffer = kmalloc(count + 1, GFP_KERNEL);
		if (buffer != NULL){
			count = vsnprintf(buffer, count + 1, format, copy);
			if(count < 0){
				kfree(buffer);
			}else{
				*ret = buffer;
			}
		}
	}
	va_end(copy);

	return count;
}

int asprintf(char **ret, const char *format, ...){
	int count;
	va_list args;
	va_start(args, format);
	count = vasprintf(ret, format, args);
	va_end(args);
	return(count);
}