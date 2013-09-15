#include "hookfns.h"

static unsigned long orig_cr0;

/*****************************************************************************\
| Methods to get/set the system call table to RW or RO                        |
\*****************************************************************************/

unsigned long clear_and_return_cr0(void){
	unsigned long cr0 = 0;
	unsigned long ret;
	asm volatile("movq %%cr0, %%rax" : "=a"(cr0));
	ret = cr0;
	cr0 &= 0xfffffffffffeffff;
	asm volatile("movq %%rax, %%cr0" : : "a"(cr0));
	return ret;
}

void setback_cr0(unsigned long val){
	asm volatile("movq %%rax, %%cr0" : : "a"(val));
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

/*****************************************************************************\
| Method to get the system call table                                         |
\*****************************************************************************/

int get_sct(void){
	int ret = 1;

	sys_call_table = get_sys_call_table();
	if(sys_call_table == NULL){
		DEBUG(KERN_INFO "sys_call_table is NULL\n");
		ret = 0;
	}else{
		DEBUG(KERN_INFO "get_sct sct: %p\n", sys_call_table);
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_ia32_sys_call_table();
	if(ia32_sys_call_table == NULL){
		DEBUG(KERN_INFO "ia32_sys_call_table is NULL\n");
		ret = 0;
	}else{
		DEBUG(KERN_INFO "get_sct ia32_sct: %p\n", ia32_sys_call_table);
	}
#endif

	return ret;
}

int set_sct_rw(void){
	// NOTE: On SMP systems, there is a scheduling race that must be dealt with.
	// http://vulnfactory.org/blog/2011/08/12/wp-safe-or-not/
	preempt_disable();
	barrier();

	orig_cr0 = clear_and_return_cr0();

	return 1;
}

int set_sct_ro(void){
	setback_cr0(orig_cr0);

	barrier();
	preempt_enable_no_resched();

	return 1;
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

/*****************************************************************************\
| This is where the magic happens. We iterate over the .syscalls ELF section  |
| and get the information that was stored there by __REGISTER_SYSCALL/32.     |
| Once we have that information, we hook all the available syscalls           |
\*****************************************************************************/

void hook_calls(void){

	counter_info_t *iter;

	if(get_sct() && set_sct_rw()){

		iter = __start_syscalls;
		for(; iter < __stop_syscalls; ++iter){
			if(iter->is32){
				DEBUG(KERN_INFO "HOOK_IA32 %s\n", iter->name);
				iter->rf = (void *)ia32_sys_call_table[iter->__NR_];
				ia32_sys_call_table[iter->__NR_] = (void *)iter->ff;
			}else{
				DEBUG(KERN_INFO "HOOK %s\n", iter->name);
				iter->rf = (void *)sys_call_table[iter->__NR_];
				sys_call_table[iter->__NR_] = (void *)iter->ff;
			}
		}

		set_sct_ro();
	}
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/


/*****************************************************************************\
| This is where we restore the REAL functions, aka, undo what HOOK and        |
| HOOK_IA32 did.                                                              |
\*****************************************************************************/

void unhook_calls(void){

	counter_info_t *iter;

	if(get_sct() && set_sct_rw()){

		iter = __start_syscalls;
		for(; iter < __stop_syscalls; ++iter){
			if(iter->is32){
				DEBUG(KERN_INFO "UNHOOK_IA32 %s\n", iter->name);
				ia32_sys_call_table[iter->__NR_] = (void *)iter->rf;
			}else{
				DEBUG(KERN_INFO "UNHOOK %s\n", iter->name);
				sys_call_table[iter->__NR_] = (void *)iter->rf;
			}
		}

		set_sct_ro();
	}
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

int safe_to_unload(void){
	int sw = 1;
	counter_info_t *iter;

	iter = __start_syscalls;
	for(; iter < __stop_syscalls; ++iter){
		DEBUG(KERN_INFO "Unloading syscall %s\n", iter->name);
		if(atomic_read(&iter->counter) > 0){
			sw = 0;
		}
	}

	return sw;
}