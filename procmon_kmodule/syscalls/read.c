#include "read.h"

REGISTER_SYSCALL(read);
REGISTER_SYSCALL32(read);

asmlinkage ssize_t (*real_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage ssize_t hooked_sys_read(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_info *i;

	__INCR(read);

	r = (*real_sys_read)(fd, buf, count);

	if(!is_active()){

		__DECR(read);

		return r;

	}else{

		deactivate();

		i = kmalloc(sizeof(syscall_info), GFP_KERNEL);
		if(i){
			i->pname = current->comm;
			i->pid = current->pid;
			i->operation = "READ";
			i->path = path_from_fd(fd);

			if(IS_ERR((void *)r)){
				i->result = "Error";
				i->details = kasprintf(GFP_KERNEL, "Errno %zd", r);
			}else{
				i->result = "Ok";
				i->details = kasprintf(GFP_KERNEL, "Read %zd bytes (was requested to read %zd)", r, count);
			}

			print_info(i);

			kfree(i->path);
			kfree(i->details);
			kfree(i);
		}else{
			//something bad happened, can't show results
		}

		activate();

		__DECR(read);

		return r;

	}
}

#ifdef CONFIG_IA32_EMULATION
asmlinkage ssize_t (*real_sys32_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage ssize_t hooked_sys32_read(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_info *i;

	__INCR32(read);

	r = (*real_sys32_read)(fd, buf, count);

	if(!is_active()){

		__DECR32(read);

		return r;

	}else{

		deactivate();

		i = kmalloc(sizeof(syscall_info), GFP_KERNEL);
		if(i){
			i->pname = current->comm;
			i->pid = current->pid;
			i->operation = "READ32";
			i->path = path_from_fd(fd);

			if(IS_ERR((void *)r)){
				i->result = "Error";
				i->details = kasprintf(GFP_KERNEL, "Errno %zd", r);
			}else{
				i->result = "Ok";
				i->details = kasprintf(GFP_KERNEL, "Read %zd bytes (was requested to read %zd)", r, count);
			}

			if(count == 1 && fd == 0)
				print_info(i);

			kfree(i->path);
			kfree(i->details);
			kfree(i);
		}else{
			//something bad happened, can't show results
		}

		activate();

		__DECR32(read);

		return r;
	}
}
#endif
