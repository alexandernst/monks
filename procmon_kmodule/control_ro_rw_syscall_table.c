#include "control_ro_rw_syscall_table.h"

static unsigned long orig_cr0;

/*****************************************************************************\
| Methods to get/set the system call table to RW or RO                         |
\*****************************************************************************/

//Method 1
void *get_writable_sct(void *sct_addr){
	struct page *p[2];
	void *sct;
	unsigned long addr = (unsigned long)sct_addr & PAGE_MASK;

	if(sct_addr == NULL)
		return NULL;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22) && defined(__x86_64__)
	p[0] = pfn_to_page(__pa_symbol(addr) >> PAGE_SHIFT);
	p[1] = pfn_to_page(__pa_symbol(addr + PAGE_SIZE) >> PAGE_SHIFT);
#else
	p[0] = virt_to_page(addr);
	p[1] = virt_to_page(addr + PAGE_SIZE);
#endif
	sct = vmap(p, 2, VM_MAP, PAGE_KERNEL);
	return sct == NULL ? NULL : sct + offset_in_page(sct_addr);
}

//Method 2
int make_rw(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	if(pte->pte &~ _PAGE_RW)
		pte->pte |= _PAGE_RW;
	return 0;
}

int make_ro(unsigned long address){
	unsigned int level;
	pte_t *pte = lookup_address(address, &level);
	pte->pte = pte->pte &~ _PAGE_RW;
	return 0;
}

//Method 3
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

//General
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
#if method == 1
	sys_call_table = get_writable_sct(sys_call_table);
	DEBUG(KERN_INFO "set_sct_rw method 1: %p\n", sys_call_table);
#elif method == 2
	make_rw((unsigned long)sys_call_table);
	DEBUG(KERN_INFO "set_sct_rw method 2: %p\n", sys_call_table);
#elif method == 3
	/*
	 * NOTE: On SMP systems, there is a scheduling race that must be dealt with.
	 *       http://vulnfactory.org/blog/2011/08/12/wp-safe-or-not/
	 */
	preempt_disable();
	barrier();

	orig_cr0 = clear_and_return_cr0(); //call only once for both archs
	DEBUG(KERN_INFO "set_sct_rw method 3 cr0: %lu\n", orig_cr0);
#endif
#ifdef CONFIG_IA32_EMULATION
#if method == 1
	ia32_sys_call_table = get_writable_sct(ia32_sys_call_table);
	DEBUG(KERN_INFO "set_sct_rw ia32_method 1: %p\n", ia32_sys_call_table);
#elif method == 2
	make_rw((unsigned long)ia32_sys_call_table);
	DEBUG(KERN_INFO "set_sct_rw ia32_method 2: %p\n", ia32_sys_call_table);
#endif
#endif

	return 1;
}

int set_sct_ro(void){
#if method == 1
	DEBUG(KERN_INFO "set_sct_ro method 1: %p\n", sys_call_table);
	vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
#elif method == 2
	DEBUG(KERN_INFO "set_sct_ro method 2: %p\n", sys_call_table);
	make_ro((unsigned long)sys_call_table);
#elif method == 3
	DEBUG(KERN_INFO "set_sct_ro method 3 cr0: %lu\n", orig_cr0);
	setback_cr0(orig_cr0); //call only once for both archs

	barrier();
	preempt_enable_no_resched();
#endif
#ifdef CONFIG_IA32_EMULATION
#if method == 1
	DEBUG(KERN_INFO "set_sct_ro ia32_method 1: %p\n", ia32_sys_call_table);
	vunmap((void*)((unsigned long)ia32_sys_call_table & PAGE_MASK));
#elif method == 2
	DEBUG(KERN_INFO "set_sct_ro ia32_method 2: %p\n", ia32_sys_call_table);
	make_ro((unsigned long)ia32_sys_call_table);
#endif
#endif

	return 1;
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/