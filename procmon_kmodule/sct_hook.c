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
|                                      END                                    |
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
	void *addr;
	unsigned char *bytecode;

	#ifdef __i386__
		return (void *)iter->ff;
	#else

		unsigned char opcode[] = {
			0x55,                                                       //push rbp;
			0x48, 0x89, 0xE5,                                           //mov rbp, rsp;
			0x48, 0x83, 0xEC, 0x40,                                     //sub rsp, 64; //8 bytes for rax content + 48 bytes for 6 args + 8 bytes for syscall result

			0x48, 0x89, 0x45, 0xF8,                                     //mov [rbp - 8], rax;
			0x48, 0x89, 0x7D, 0xF0,                                     //mov [rbp - 16], rdi;
			0x48, 0x89, 0x75, 0xE8,                                     //mov [rbp - 24], rsi;
			0x48, 0x89, 0x55, 0xE0,                                     //mov [rbp - 32], rdx;
			0x48, 0x89, 0x4D, 0xD8,                                     //mov [rbp - 40], rcx;
			0x4C, 0x89, 0x45, 0xD0,                                     //mov [rbp - 48], r8;
			0x4C, 0x89, 0x4D, 0xC8,                                     //mov [rbp - 56], r9;

/*38*/		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, &atomic_inc;
/*48*/		0x48, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, &iter->counter;
			0xFF, 0xD0,                                                 //call rax;

/*60*/		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, &iter->rf;
			0x48, 0x8B, 0x7D, 0xF0,                                     //mov rdi, [rbp - 16];
			0x48, 0x8B, 0x75, 0xE8,                                     //mov rsi, [rbp - 24];
			0x48, 0x8B, 0x55, 0xE0,                                     //mov rdx, [rbp - 32];
			0x48, 0x8B, 0x4D, 0xD8,                                     //mov rcx, [rbp - 40];
			0x4C, 0x8B, 0x45, 0xD0,                                     //mov r8, [rbp - 48];
			0x4C, 0x8B, 0x4D, 0xC8,                                     //mov r9, [rbp - 56];
			0xFF, 0xD0,                                                 //call rax;

			0x48, 0x89, 0x45, 0xC0,                                     //mov [rbp - 64], rax;

			//TODO: Call iter->ff only if procmon_state == 1 and iter->state == 1
/*100*/		0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, [&procmon_state];
			0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00,                   //mov rdi, 1;
			0x48, 0x39, 0xF8,                                           //cmp rax, rdi;
			0x75, 0x3A,                                                 //jne $+2+22+36;

/*122*/		0x48, 0xA1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, [iter->state];
			0x48, 0xC7, 0xC7, 0x01, 0x00, 0x00, 0x00,                   //mov rdi, 1;
			0x48, 0x39, 0xF8,                                           //cmp rax, rdi;
			0x75, 0x24,                                                 //jne $+2+36;

/*144*/		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, &iter->ff;
			0x48, 0x8B, 0x7D, 0xF0,                                     //mov rdi, [rbp - 16];
			0x48, 0x8B, 0x75, 0xE8,                                     //mov rsi, [rbp - 24];
			0x48, 0x8B, 0x55, 0xE0,                                     //mov rdx, [rbp - 32];
			0x48, 0x8B, 0x4D, 0xD8,                                     //mov rcx, [rbp - 40];
			0x4C, 0x8B, 0x45, 0xD0,                                     //mov r8, [rbp - 48];
			0x4C, 0x8B, 0x4D, 0xC8,                                     //mov r9, [rbp - 56];
			0xFF, 0xD0,                                                 //call rax;

/*180*/		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, &atomic_dec;
/*190*/		0x48, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, &iter->counter;
			0xFF, 0xD0,                                                 //call rax;

			0x48, 0x8B, 0x45, 0xC0,                                     //mov rax, [rbp - 64];

			0x48, 0x89, 0xEC,                                           //mov rsp, rbp;
			0x5D,                                                       //pop rbp;
			0xC3                                                        //ret;
		};

		bytecode = __vmalloc(sizeof(opcode), GFP_KERNEL, PAGE_KERNEL_EXEC);

		//patch addrs
		addr = &atomic_inc;
		memcpy(opcode + 38, &addr, sizeof(void *)); //&atomic_inc
		memcpy(opcode + 48, &iter->counter, sizeof(void *)); //&iter->counter
		memcpy(opcode + 60, &iter->rf, sizeof(void *)); //&iter->rf
		addr = &procmon_state;
		memcpy(opcode + 100, &addr, sizeof(void *)); //&procmon_state
		addr = &iter->state;
		memcpy(opcode + 122, &addr, sizeof(void *)); //&iter->state
		memcpy(opcode + 144, &iter->ff, sizeof(void *)); //&iter->ff
		addr = &atomic_dec;
		memcpy(opcode + 180, &addr, sizeof(void *)); //&atomic_dec
		memcpy(opcode + 190, &iter->counter, sizeof(void *)); //&iter->counter
		
		memcpy(bytecode, opcode, sizeof(opcode));

/*
		printk("&iter->ff: %p\n", (void *)iter->ff);
		printk("Bytecode: \n");
		for(i = 0; i < sizeof(opcode); ++i){
			printk("%02x", bytecode[i]);
		}
		printk("\nEnd\n");
*/
		return bytecode;
		//return (void *)iter->rf;

	#endif
}

#ifdef CONFIG_IA32_EMULATION
static void *create_stub32(syscall_info_t *iter){
	return (void *)iter->ff;
}
#endif

/*****************************************************************************\
|                                      END                                    |
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
|                                      END                                    |
\*****************************************************************************/

#define __NR_syscall_max	512	/* TODO: find out real value */

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

		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			procmon_info("Hook IA32 %s\n", iter->name);
			iter->rf = (void *)ia32_sct_map[iter->__NR_];
			ia32_sct_map[iter->__NR_] = create_stub32(iter);
#endif
		}else{
			procmon_info("Hook %s\n", iter->name);
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
#ifdef CONFIG_IA32_EMULATION
	vunmap((void *)((unsigned long)ia32_sct_map & PAGE_MASK)), ia32_sct_map = NULL;
#endif
	vunmap((void *)((unsigned long)sct_map & PAGE_MASK)), sct_map = NULL;
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

/*****************************************************************************\
| This is where we restore the REAL functions, aka, undo what HOOK and        |
| HOOK_IA32 did.                                                              |
\*****************************************************************************/

static int do_unhook_calls(void *arg){
	syscall_info_t *iter;

	for(iter = __start_syscalls; iter < __stop_syscalls; ++iter){
		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			procmon_info("Unhook IA32 %s\n", iter->name);
			ia32_sct_map[iter->__NR_] = destroy_stub32(iter);
#endif
		}else{
			procmon_info("Unhook %s\n", iter->name);
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
#ifdef CONFIG_IA32_EMULATION
	vunmap((void *)((unsigned long)ia32_sct_map & PAGE_MASK)), ia32_sct_map = NULL;
#endif
	vunmap((void *)((unsigned long)sct_map & PAGE_MASK)), sct_map = NULL;
}

/*****************************************************************************\
|                                      END                                    |
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
