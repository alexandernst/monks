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

#define __NR_syscall_max	512	/* TODO: find out real value */

/*****************************************************************************\
| This is where the magic happens. We iterate over the .syscalls ELF section  |
| and get the information that was stored there by __REGISTER_SYSCALL/32.     |
| Once we have that information, we hook all the available syscalls           |
\*****************************************************************************/

static int do_hook_calls(void * arg)
{
	syscall_info_t *iter;

	for (iter = __start_syscalls; iter < __stop_syscalls; iter++) {
		if (iter->is32) {
#ifdef CONFIG_IA32_EMULATION
			procmon_info("Hook IA32 %s\n", iter->name);
			iter->rf = (void *)ia32_sct_map[iter->__NR_];
			ia32_sct_map[iter->__NR_] = (void *)iter->ff;
#endif
		} else {
			procmon_info("Hook %s\n", iter->name);
			iter->rf = (void *)sct_map[iter->__NR_];
			sct_map[iter->__NR_] = (void *)iter->ff;
		}

		add_syscalls_state_table_entry(iter->name, &iter->state);
		iter->counter = new(sizeof(atomic_t));
		atomic_set(iter->counter, 0);
	}

	return 0;
}

void hook_calls(void)
{
	if (!get_sct())
		return;

	sct_map = map_writable(sys_call_table, __NR_syscall_max * sizeof(void *));
	if (!sct_map) {
		procmon_error("Can't get writable SCT mapping\n");
		goto out;
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sct_map = map_writable(ia32_sys_call_table, __NR_syscall_max * sizeof(void *));
	if (!ia32_sct_map) {
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

static int do_unhook_calls(void * arg)
{
	syscall_info_t *iter;

	for(iter = __start_syscalls; iter < __stop_syscalls; ++iter) {
		if(iter->is32){
#ifdef CONFIG_IA32_EMULATION
			procmon_info("Unhook IA32 %s\n", iter->name);
			ia32_sct_map[iter->__NR_] = (void *)iter->rf;
#endif
		} else {
			procmon_info("Unhook %s\n", iter->name);
			sct_map[iter->__NR_] = (void *)iter->rf;
		}
	}

	return 0;
}

void unhook_calls(void)
{
	if (!get_sct())
		return;

	sct_map = map_writable(sys_call_table, __NR_syscall_max * sizeof(void *));
	if (!sct_map) {
		procmon_error("Can't get writable SCT mapping\n");
		goto out;
	}

#ifdef CONFIG_IA32_EMULATION
	ia32_sct_map = map_writable(ia32_sys_call_table, __NR_syscall_max * sizeof(void *));
	if (!ia32_sct_map) {
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
