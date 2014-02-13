#include "sct_hook.h"

void **sys_call_table = NULL;
static void **sct_map = NULL;
#ifdef CONFIG_IA32_EMULATION
void **ia32_sys_call_table = NULL;
static void **ia32_sct_map = NULL;
#endif

/*****************************************************************************\
| Functions to get the address of the system call table on each arch          |
\*****************************************************************************/

#if defined(CONFIG_X86_32) || defined(CONFIG_IA32_EMULATION)
#ifdef CONFIG_X86_32
void *get_sys_call_table(void){
#elif defined(CONFIG_IA32_EMULATION)
void *get_ia32_sys_call_table(void){
#endif
	struct idtr idtr;
	struct idt_descriptor idtd;
	void *system_call;

	asm volatile("sidt %0" : "=m" (idtr));
	memcpy(&idtd, idtr.base + 0x80 * sizeof(idtd), sizeof(idtd));

#ifdef CONFIG_X86_32
	system_call = (void *)((idtd.offset_high << 16) | idtd.offset_low);
#elif defined(CONFIG_IA32_EMULATION)
	system_call = (void *)(((long)idtd.offset_high << 32) | (idtd.offset_middle << 16) | idtd.offset_low);
#endif

	return ud_find_syscall_table_addr(system_call);
}
#endif

#ifdef CONFIG_X86_64
void *get_sys_call_table(void){
	int low, high;
	void *system_call;

	asm volatile("rdmsr" : "=a" (low), "=d" (high) : "c" (0xc0000082));
	system_call = (void *)(((u64)high << 32) | low);

	return ud_find_syscall_table_addr(system_call);
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
		monks_info("syscall_table is NULL, quitting...\n");
		return 0;
	}else{
		monks_info("Found syscall_table addr at 0x%p\n", sys_call_table);
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_ia32_sys_call_table();
	if(!ia32_sys_call_table){
		monks_info("syscall_table is NULL, quitting...\n");
		return 0;
	}else{
		monks_info("Found ia32_syscall_table addr at 0x%p\n", ia32_sys_call_table);
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
| and filled with opcode. Also, the stub will survive monks's unloading.      |
|                                                                             |
| The stub will do the following things:                                      |
|                                                                             |
| 1. Save syscall arguments                                                   |
| 2. Call the real syscall                                                    |
| 3. Save the returned value                                                  |
| 4. Maybe call the hooked syscall with the arguments that were saved earlier |
| 5. Return the result from the call to the real syscall                      |
\*****************************************************************************/

static void *create_stub(syscall_info_t *iter, void *stub){
	uint64_t stub_size;
	unsigned char *bytecode;

	stub_size = ud_get_stub_size(stub);
	bytecode = __vmalloc(stub_size, GFP_KERNEL, PAGE_KERNEL_EXEC);
	memcpy(bytecode, stub, stub_size);

	//patch addrs
	ud_patch_addr(bytecode, &monks_state);
	ud_patch_addr(bytecode, &iter->state);
	ud_patch_addr(bytecode, iter->pre);
	ud_patch_addr(bytecode, iter->rf);
	ud_patch_addr(bytecode, &monks_state);
	ud_patch_addr(bytecode, &iter->state);
	ud_patch_addr(bytecode, iter->post);

	return bytecode; //iter->rf;
}

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| Destroy a stub will actually change the stub's opcode so it will call only  |
| the real syscall.                                                           |
| That means that the content of the stub will look like the following:       |
|                                                                             |
| 1. Save syscall arguments                                                   |
| 2. Call the real syscall                                                    |
| 3. Save the returned value                                                  |
| 4. Return the result from the call to the real syscall                      |
\*****************************************************************************/

static void *destroy_stub(syscall_info_t *iter, void *stub){
	ud_patch_cmp(stub);

	return stub; //iter->rf;
}

/*****************************************************************************\
|                                     END                                     |
\*****************************************************************************/

/*****************************************************************************\
| This is where the magic happens. We iterate over the .syscalls ELF section  |
| and get the information that was stored there by __REGISTER_SYSCALL/32.     |
| Once we have that information, we hook all the available syscalls           |
\*****************************************************************************/

static int do_hook_calls(void *arg){
	extern void *stub;
#ifdef CONFIG_IA32_EMULATION
	extern void *stub_32;
#endif	
	syscall_info_t *iter;

	for_each_syscall(iter){

		add_syscalls_state_table_entry(iter->name, &iter->state);
		monks_info("Hook %s\n", iter->name);

		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			iter->rf = (void *)ia32_sct_map[iter->__NR_];
			ia32_sct_map[iter->__NR_] = create_stub(iter, &stub_32);
#endif
		}else{
			iter->rf = (void *)sct_map[iter->__NR_];
			sct_map[iter->__NR_] = create_stub(iter, &stub);
		}
	}

	return 0;
}

void hook_calls(void){
	if(!get_sct())
		return;

	sct_map = map_writable(sys_call_table, __NR_syscall_max * sizeof(void *));
	if(!sct_map){
		monks_error("Can't get writable SCT mapping\n");
		goto out;
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sct_map = map_writable(ia32_sys_call_table, __NR32_syscall_max * sizeof(void *));
	if(!ia32_sct_map){
		monks_error("Can't get writable IA32_SCT mapping\n");
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

	for_each_syscall(iter){

		monks_info("Unhook %s\n", iter->name);
		
		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			ia32_sct_map[iter->__NR_] = destroy_stub(iter, ia32_sct_map[iter->__NR_]);
#endif
		}else{
			sct_map[iter->__NR_] = destroy_stub(iter, sct_map[iter->__NR_]);
		}
	}

	return 0;
}

void unhook_calls(void){
	if(!get_sct())
		return;

	sct_map = map_writable(sys_call_table, __NR_syscall_max * sizeof(void *));
	if(!sct_map){
		monks_error("Can't get writable SCT mapping\n");
		goto out;
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sct_map = map_writable(ia32_sys_call_table, __NR32_syscall_max * sizeof(void *));
	if(!ia32_sct_map){
		monks_error("Can't get writable IA32_SCT mapping\n");
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
