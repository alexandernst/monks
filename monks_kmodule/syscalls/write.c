#include "syscall.h"

extern asmlinkage void hooked_sys_pre_write(unsigned int fd, char __user *buf, size_t count);
extern asmlinkage void hooked_sys_post_write(unsigned int fd, char __user *buf, size_t count);
#ifdef CONFIG_IA32_EMULATION
extern asmlinkage void hooked_sys32_pre_write(unsigned int fd, char __user *buf, size_t count);
extern asmlinkage void hooked_sys32_post_write(unsigned int fd, char __user *buf, size_t count);
#endif

__REGISTER_SYSCALL(write);

asmlinkage void hooked_sys_pre_write(unsigned int fd, char __user *buf, size_t count){
	syscall_intercept_info *i;

	i = new(sizeof(struct syscall_intercept_info));

	__SET_SYSCALL_HOOK_INFO(i);

	if(!i)
		return;

	i->pname = current->comm;
	i->pid = current->pid;
	i->operation = "WRITE";
	i->path = path_from_fd(fd);

	return;
}

asmlinkage void hooked_sys_post_write(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_intercept_info *i;

	__GET_SYSCALL_RESULT(r);
	__GET_SYSCALL_HOOK_INFO(i);

	if(!i)
		return;

	if(IS_ERR((void *)r)){
		i->result = "Error";
		i->details = kasprintf(GFP_KERNEL, "Errno %zd", r);
	}else{
		i->result = "Ok";
		i->details = kasprintf(GFP_KERNEL, "Write %zd bytes (was requested to write %zd)", r, count);
	}

	nl_send(i);

	del(i->path);
	del(i->details);
	del(i);

	return;
}

#ifdef CONFIG_IA32_EMULATION
__REGISTER_SYSCALL32(write);

asmlinkage void hooked_sys32_pre_write(unsigned int fd, char __user *buf, size_t count){
	syscall_intercept_info *i;

	i = new(sizeof(struct syscall_intercept_info));

	__SET_SYSCALL_HOOK_INFO32(i);

	if(!i)
		return;

	i->pname = current->comm;
	i->pid = current->pid;
	i->operation = "WRITE32";
	i->path = path_from_fd(fd);

	return;
}

asmlinkage void hooked_sys32_post_write(unsigned int fd, char __user *buf, size_t count){
	ssize_t r;
	syscall_intercept_info *i;

	__GET_SYSCALL_RESULT32(r);
	__GET_SYSCALL_HOOK_INFO32(i);

	if(!i)
		return;

	if(IS_ERR((void *)r)){
		i->result = "Error";
		i->details = kasprintf(GFP_KERNEL, "Errno %zd", r);
	}else{
		i->result = "Ok";
		i->details = kasprintf(GFP_KERNEL, "Write %zd bytes (was requested to write %zd)", r, count);
	}

	nl_send(i);

	del(i->path);
	del(i->details);
	del(i);

	return;
}

#endif
