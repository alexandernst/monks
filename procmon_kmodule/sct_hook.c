#include "sct_hook.h"

#define __NR_syscall_max	512	/* TODO: find out real value */

void **sys_call_table = NULL;
static void **sct_map = NULL;
#ifdef CONFIG_IA32_EMULATION
void **ia32_sys_call_table = NULL;
static void **ia32_sct_map = NULL;
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
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| Method to get the system call table                                         |
\*****************************************************************************/

static int get_sct(void){
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

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| Create a stub that will replace the real syscall.                           |
| The stub is just a chunk of executable memory, kmalloc-ed from the module   |
| and filled with opcode. Also, the stub will survive procmon's unloading.    |
|                                                                             |
| The stub will do the following things:                                      |
|                                                                             |
| 1. Save syscall arguments                                                   |
| 2. Call atomic_inc with the counter of the current hijacked syscall         |
| 3. Call the real syscall                                                    |
| 4. Save the returned value                                                  |
| 5. Call the hooked syscall with the arguments that were saved earlier       |
| 6. Call atomic_dec with the counter of the current hijacked syscall         |
| 7. Return the result from the call to the real syscall                      |
\*****************************************************************************/

static void *create_stub(syscall_info_t *iter){
	int i;
	extern void *stub;
	uint64_t stub_size;
	unsigned char *bytecode;

	stub_size = ud_get_stub_size(&stub);
	bytecode = __vmalloc(stub_size, GFP_KERNEL, PAGE_KERNEL_EXEC);
	memcpy(bytecode, &stub, stub_size);

	//patch addrs
	ud_patch_addr(bytecode, iter->counter);
	ud_patch_addr(bytecode, iter->rf);
	ud_patch_addr(bytecode, &procmon_state);
	ud_patch_addr(bytecode, &iter->state);
	ud_patch_addr(bytecode, iter->ff);
	ud_patch_addr(bytecode, iter->counter);

	printk("&iter->counter: %p\n", iter->counter);
	printk("&iter->rf: %p\n", iter->rf);
	printk("&procmon_state: %p\n", &procmon_state);
	printk("&iter->state: %p\n", &iter->state);
	printk("&iter->ff: %p\n", iter->ff);

	printk("Bytecode: \n");
	for(i = 0; i < stub_size; ++i){
		printk("%02x", bytecode[i]);
	}
	printk("\nEnd\n");

	//return (void *)iter->rf;

	return bytecode;
}

#ifdef CONFIG_IA32_EMULATION
static void *create_stub32(syscall_info_t *iter){
	//TODO
	return (void *)iter->rf;
}
#endif

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| Destroy a stub will actually change the stub's opcode so it will destroy    |
| itself when there are no more sleeping calls that will use the stub.        |
|                                                                             |
| That means that the content of the stub will look like the following:       |
|                                                                             |
| 1. Save syscall arguments                                                   |
| 2. Call atomic_inc with the counter of the current hijacked syscall         |
| 3. Call the real syscall                                                    |
| 4. Save the returned value                                                  |
| 5. Call atomic_dec with the counter of the current hijacked syscall         |
| 6. If the atomic_counter is equal to 0 then make the stub free itself       |
| 7. Return the result from the call to the real syscall                      |
\*****************************************************************************/

static void *destroy_stub(syscall_info_t *iter){
	return (void *)iter->rf;
}

#ifdef CONFIG_IA32_EMULATION
static void *destroy_stub32(syscall_info_t *iter){
	return (void *)iter->rf;
}
#endif

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| This is where the magic happens. We iterate over the .syscalls ELF section  |
| and get the information that was stored there by __REGISTER_SYSCALL/32.     |
| Once we have that information, we hook all the available syscalls           |
\*****************************************************************************/

static int do_hook_calls(void *arg){
	syscall_info_t *iter;

	for(iter = __start_syscalls; iter < __stop_syscalls; iter++){
		add_syscalls_state_table_entry(iter->name, &iter->state);
		iter->counter = new(sizeof(atomic_t));
		atomic_set(iter->counter, 0);

		procmon_info("Hook %s\n", iter->name);
		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			iter->rf = (void *)ia32_sct_map[iter->__NR_];
			ia32_sct_map[iter->__NR_] = create_stub32(iter);
#endif
		}else{
			iter->rf = (void *)sct_map[iter->__NR_];
			sct_map[iter->__NR_] = create_stub(iter);
		}
	}

	return 0;
}

void hook_calls(void){
	if(!get_sct())
		return;

	sct_map = map_writable(sys_call_table, __NR_syscall_max * sizeof(void *));
	if(!sct_map){
		procmon_error("Can't get writable SCT mapping\n");
		goto out;
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sct_map = map_writable(ia32_sys_call_table, __NR_syscall_max * sizeof(void *));
	if(!ia32_sct_map){
		procmon_error("Can't get writable IA32_SCT mapping\n");
		goto out;
	}
#endif

	/* stop the fucking machine */
	stop_machine(do_hook_calls, NULL, 0);

out:
	vunmap((void *)((unsigned long)sct_map & PAGE_MASK)), sct_map = NULL;
#ifdef CONFIG_IA32_EMULATION
	vunmap((void *)((unsigned long)ia32_sct_map & PAGE_MASK)), ia32_sct_map = NULL;
#endif
}

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| This is where we restore the REAL functions, aka, undo what HOOK and        |
| HOOK_IA32 did.                                                              |
\*****************************************************************************/

static int do_unhook_calls(void *arg){
	syscall_info_t *iter;

	for(iter = __start_syscalls; iter < __stop_syscalls; ++iter){
		procmon_info("Unhook %s\n", iter->name);
		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			ia32_sct_map[iter->__NR_] = destroy_stub32(iter);
#endif
		}else{
			sct_map[iter->__NR_] = destroy_stub(iter);
		}
	}

	return 0;
}

void unhook_calls(void){
	if(!get_sct())
		return;

	sct_map = map_writable(sys_call_table, __NR_syscall_max * sizeof(void *));
	if(!sct_map){
		procmon_error("Can't get writable SCT mapping\n");
		goto out;
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sct_map = map_writable(ia32_sys_call_table, __NR_syscall_max * sizeof(void *));
	if(!ia32_sct_map){
		procmon_error("Can't get writable IA32_SCT mapping\n");
		goto out;
	}
#endif

	/* stop the fucking machine */
	stop_machine(do_unhook_calls, NULL, 0);

out:
	vunmap((void *)((unsigned long)sct_map & PAGE_MASK)), sct_map = NULL;
#ifdef CONFIG_IA32_EMULATION
	vunmap((void *)((unsigned long)ia32_sct_map & PAGE_MASK)), ia32_sct_map = NULL;
#endif
}

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

int safe_to_unload(void){
	syscall_info_t *iter = __start_syscalls;

	for(; iter < __stop_syscalls; ++iter){
		procmon_info("Unloading syscall %s\n", iter->name);
		if(iter->counter && atomic_read(iter->counter) > 0){
			return 0;
		}else if(iter->counter && atomic_read(iter->counter) == 0){
			del(iter->counter);
		}
	}

	return 1;
}
