#include "syshijack.h"

asmlinkage long (*real_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hooked_sys_read(unsigned int fd, char __user *buf, size_t count){
	if(count == 1 && fd == 0)
		printk(KERN_INFO "intercepted sys_read from %s\n", current->comm);
	return real_sys_read(fd, buf, count);
}

void hook_calls(void){
	sys_call_table = get_writable_sct(get_sys_call_table());
	if(sys_call_table == NULL){
		printk(KERN_INFO "sys_call_table is NULL\n");
		return;
	}
#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_writable_sct(get_ia32_sys_call_table());
	if(ia32_sys_call_table == NULL){
		vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
		printk(KERN_INFO "ia32_sys_call_table is NULL\n");
		return;
	}
#endif


#ifdef __NR_read32
#define F __NR_read32
#else
#define F __NR_read
#endif
HOOK(F); HOOK_IA32(F);
#undef F


}

void unhook_calls(void){

#ifdef __NR_read32
#define F __NR_read32
#else
#define F __NR_read
#endif
UNHOOK(F); UNHOOK_IA32(F);
#undef F


	vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
#ifdef CONFIG_IA32_EMULATION
	vunmap((void*)((unsigned long)ia32_sys_call_table & PAGE_MASK));
#endif
}