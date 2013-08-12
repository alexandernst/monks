#include "fns.h"

/*****************************************************************************\
| (For each syscall) Define a function which we will use to save the REAL     |
| syscall function and define a FAKE function which we will use to replace    |
| the REAL one.                                                               |
| Also, do that for x86 and x64 cases, AND for the special IA32 case.         |
| A list of all syscalls can be retrieved using man (man syscalls).           |
\*****************************************************************************/

/* __NR_read / __NR32_read */
asmlinkage long (*real_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hooked_sys_read(unsigned int fd, char __user *buf, size_t count){

	INCR_SYSCALL_REG_INFO(__NR_read);

	ssize_t r;
	r = (*real_sys_read)(fd, buf, count);

	if(!is_active()){

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

		return r;

	}
}

#ifdef CONFIG_IA32_EMULATION
asmlinkage long (*real_sys_read32)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hooked_sys_read32(unsigned int fd, char __user *buf, size_t count){

	INCR_SYSCALL_REG_INFO(__NR32_read);

	ssize_t r;
	r = (*real_sys_read32)(fd, buf, count);

	if(!is_active()){

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

		return r;
	}
}
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/