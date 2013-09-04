#include "read.h"

asmlinkage ssize_t (*real_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage ssize_t hooked_sys_read(unsigned int fd, char __user *buf, size_t count){

	__INCR(read);

	ssize_t r;
	r = (*real_sys_read)(fd, buf, count);

	if(!is_active()){

		__DECR(read);

		return r;

	}else{

		deactivate();

		syscall_info *i = kmalloc(sizeof(syscall_info), GFP_KERNEL);

		i->pname = current->comm;
		i->pid = current->pid;
		i->operation = "READ";
		i->path = path_from_fd(fd);

		char s[256];
		if(IS_ERR((void *)r)){
			i->result = "Error";
			sprintf(s, "Errno %zd", r);
		}else{
			i->result = "Ok";
			sprintf(s, "Read %zd bytes (was requested to read %zd)", r, count);
		}
		i->details = s;

		print_info(i);

		kfree(i->path);
		kfree(i);

		activate();

		__DECR(read);

		return r;

	}
}

#ifdef CONFIG_IA32_EMULATION
asmlinkage ssize_t (*real_sys32_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage ssize_t hooked_sys32_read(unsigned int fd, char __user *buf, size_t count){

	__INCR32(read);

	ssize_t r;
	r = (*real_sys32_read)(fd, buf, count);

	if(!is_active()){

		__DECR32(read);

		return r;

	}else{

		deactivate();

		syscall_info *i = kmalloc(sizeof(syscall_info), GFP_KERNEL);

		i->pname = current->comm;
		i->pid = current->pid;
		i->operation = "READ32";
		i->path = path_from_fd(fd);

		char s[256];
		if(IS_ERR((void *)r)){
			i->result = "Error";
			sprintf(s, "Errno %zd", r);
		}else{
			i->result = "Ok";
			sprintf(s, "Read %zd bytes (was requested to read %zd)", r, count);
		}
		i->details = s;

		if(count == 1 && fd == 0)
			print_info(i);

		kfree(i->path);
		kfree(i);

		activate();

		__DECR32(read);

		return r;
	}
}
#endif
