#include "hookfns.h"

static unsigned long orig_cr0;

void **sys_call_table = NULL;
#ifdef CONFIG_IA32_EMULATION
void **ia32_sys_call_table = NULL;
#endif

/*****************************************************************************\
| Functions to get the address of the system call table on each arch          |
\*****************************************************************************/

#if defined(__i386__) || defined(CONFIG_IA32_EMULATION)
#ifdef __i386__
void *get_sys_call_table(void){
#elif defined(__x86_64__)
void *get_ia32_sys_call_table(void){
#endif
	struct idtr idtr;
	struct idt_descriptor idtd;
	void *system_call;
	unsigned char *ptr;
	int i;

	asm volatile("sidt %0" : "=m" (idtr));
	memcpy(&idtd, idtr.base + 0x80 * sizeof(idtd), sizeof(idtd));

#ifdef __i386__
	system_call = (void*)((idtd.offset_high<<16) | idtd.offset_low);
	for(ptr=system_call, i=0; i<500; i++){
		if(ptr[0] == 0xff && ptr[1] == 0x14 && ptr[2] == 0x85)
			return *((void**)(ptr+3));
		ptr++;
	}
#elif defined(__x86_64__)
	system_call = (void*)(((long)idtd.offset_high<<32) | (idtd.offset_middle<<16) | idtd.offset_low);
	for(ptr=system_call, i=0; i<500; i++){
		if(ptr[0] == 0xff && ptr[1] == 0x14 && ptr[2] == 0xc5)
			return (void*)(0xffffffff00000000 | *((unsigned int*)(ptr+3)));
		ptr++;
	}
#endif

	return NULL;
}
#endif

#ifdef __x86_64__
void *get_sys_call_table(void){
	void *system_call;
	unsigned char *ptr;
	int i, low, high;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (0xc0000082)); //IA32_LSTAR
	system_call = (void*)(((long)high<<32) | low);

	for(ptr=system_call, i=0; i<500; i++){
		if(ptr[0] == 0xff && ptr[1] == 0x14 && ptr[2] == 0xc5)
			return (void*)(0xffffffff00000000 | *((unsigned int*)(ptr+3)));
		ptr++;
	}

	return NULL;
}
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

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
	sys_call_table = get_sys_call_table();
	if(!sys_call_table){
		DEBUG(KERN_INFO "sys_call_table is NULL\n");
		return 0;
	}else{
		DEBUG(KERN_INFO "get_sct sct: %p\n", sys_call_table);
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_ia32_sys_call_table();
	if(!ia32_sys_call_table){
		DEBUG(KERN_INFO "ia32_sys_call_table is NULL\n");
		return 0;
	}else{
		DEBUG(KERN_INFO "get_sct ia32_sct: %p\n", ia32_sys_call_table);
	}
#endif

	return 1;
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
	syscall_info_t *iter;

	if(get_sct() && set_sct_rw()){

		iter = __start_syscalls;
		for(; iter < __stop_syscalls; ++iter){
			if(iter->is32){
				DEBUG(KERN_INFO "Hook IA32 %s\n", iter->name);
				iter->rf = (void *)ia32_sys_call_table[iter->__NR_];
				ia32_sys_call_table[iter->__NR_] = (void *)iter->ff;
			}else{
				DEBUG(KERN_INFO "Hook %s\n", iter->name);
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
	syscall_info_t *iter;

	if(get_sct() && set_sct_rw()){

		iter = __start_syscalls;
		for(; iter < __stop_syscalls; ++iter){
			if(iter->is32){
				DEBUG(KERN_INFO "Unhook IA32 %s\n", iter->name);
				ia32_sys_call_table[iter->__NR_] = (void *)iter->rf;
			}else{
				DEBUG(KERN_INFO "Unhook %s\n", iter->name);
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
	syscall_info_t *iter = __start_syscalls;

	for(; iter < __stop_syscalls; ++iter){
		DEBUG(KERN_INFO "Unloading syscall %s\n", iter->name);
		if(atomic_read(&iter->counter) > 0){
			return 0;
		}
	}

	return 1;
}