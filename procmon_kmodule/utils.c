#include "utils.h"

char *path_from_fd(unsigned int fd){
	struct file *file;
	struct path path;
	struct files_struct *files = current->files;
	char *tmp, *pathname = "", *rpathname = new(1);

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
