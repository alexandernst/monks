#include "sct_hook.h"

static unsigned long orig_cr0;

void **sys_call_table = NULL;
#ifdef CONFIG_IA32_EMULATION
void **ia32_sys_call_table = NULL;
#endif

/*****************************************************************************\
| Functions to get the address of the system call table on each arch          |
\*****************************************************************************/

unsigned int ud_find_insn_arg(void *entry, int limit, enum ud_mnemonic_code insn_mne, int insn_len){
	ud_t ud;
	unsigned int result = 0;

	ud_init(&ud);
	ud_set_mode(&ud, BITS_PER_LONG);
	ud_set_vendor(&ud, UD_VENDOR_ANY);
	ud_set_input_buffer(&ud, entry, limit);

	while(ud_disassemble(&ud)){
		if(ud.mnemonic == insn_mne && ud_insn_len(&ud) == insn_len){
			return ud.operand[0].lval.sdword;
		}
	}

	return result;
}

#if defined(__i386__) || defined(CONFIG_IA32_EMULATION)
#ifdef __i386__
void *get_sys_call_table(void){
#elif defined(__x86_64__)
void *get_ia32_sys_call_table(void){
#endif
	struct idtr idtr;
	struct idt_descriptor idtd;
	void *system_call;
	unsigned int ptr;

	asm volatile("sidt %0" : "=m" (idtr));
	memcpy(&idtd, idtr.base + 0x80 * sizeof(idtd), sizeof(idtd));

#ifdef __i386__
	system_call = (void *)((idtd.offset_high << 16) | idtd.offset_low);

	ptr = ud_find_insn_arg(system_call, 512, UD_Icall, 7);
	return ptr ? to_x86_ptr(ptr) : NULL;

#elif defined(__x86_64__)
	system_call = (void *)(((long)idtd.offset_high << 32) | (idtd.offset_middle << 16) | idtd.offset_low);
	
	ptr = ud_find_insn_arg(system_call, 512, UD_Icall, 7);
	return ptr ? to_x64_ptr(ptr) : NULL;

#endif
}
#endif

#ifdef __x86_64__
void *get_sys_call_table(void){
	int low, high;
	void *system_call;
	unsigned int ptr;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (0xc0000082));
	system_call = (void *)(((u64)high << 32) | low);

	ptr = ud_find_insn_arg(system_call, 512, UD_Icall, 7);
	return ptr ? to_x64_ptr(ptr) : NULL;
}
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

/*****************************************************************************\
| Methods to get/set the system call table to RW or RO                        |
\*****************************************************************************/

unsigned long clear_and_return_cr0(void){
	unsigned long ret, cr0 = 0;
	asm volatile("mov %%cr0, %0" : "=r"(cr0));
	ret = cr0;
	cr0 &= ~(1 << 16);
	asm volatile("mov %0, %%cr0" : : "r"(cr0));
	return ret;
}

void setback_cr0(unsigned long val){
	asm volatile("mov %0, %%cr0" : : "r"(val));
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
		procmon_info("syscall_table is NULL, quitting...\n");
		return 0;
	}else{
		procmon_info("Found syscall_table addr at 0x%p\n", sys_call_table);
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_ia32_sys_call_table();
	if(!ia32_sys_call_table){
		procmon_info("syscall_table is NULL, quitting...\n");
		return 0;
	}else{
		procmon_info("Found ia32_syscall_table addr at 0x%p\n", ia32_sys_call_table);
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
#ifdef CONFIG_IA32_EMULATION
				procmon_info("Hook IA32 %s\n", iter->name);
				iter->rf = (void *)ia32_sys_call_table[iter->__NR_];
				ia32_sys_call_table[iter->__NR_] = (void *)iter->ff;
				add_syscalls_state_table_entry(iter->name, &iter->state);
#endif
			}else{
				procmon_info("Hook %s\n", iter->name);
				iter->rf = (void *)sys_call_table[iter->__NR_];
				sys_call_table[iter->__NR_] = (void *)iter->ff;
				add_syscalls_state_table_entry(iter->name, &iter->state);
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
#ifdef CONFIG_IA32_EMULATION
				procmon_info("Unhook IA32 %s\n", iter->name);
				ia32_sys_call_table[iter->__NR_] = (void *)iter->rf;
#endif
			}else{
				procmon_info("Unhook %s\n", iter->name);
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
		procmon_info("Unloading syscall %s\n", iter->name);
		if(atomic_read(&iter->counter) > 0){
			return 0;
		}
	}

	return 1;
}