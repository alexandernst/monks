#include "syshijack.h"

static char proc_data[1];
static struct proc_dir_entry *proc_write_entry;

void **sys_call_table;
#ifdef CONFIG_IA32_EMULATION
void **ia32_sys_call_table;
#endif

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

static int read_proc(char *buf, char **start, off_t offset, int count, int *eof, void *data){
	sprintf(buf, "%s\n", proc_data);
	return 0;
}

static int write_proc(struct file *file, const char __user *buf, unsigned long len, void *data){
	if(len > 2)
		return -EINVAL;

	if(copy_from_user(proc_data, buf, 1))
		return -EFAULT;

	if(strcmp("1", proc_data) == 0){
		hook_calls();
	}else if(strcmp("0", proc_data) == 0){
		unhook_calls();
	}
	
	return len;
}

static int __init hook_init(void){
	proc_write_entry = create_proc_entry("procmon", 0666, NULL);
	if(!proc_write_entry){
		printk(KERN_INFO "Error creating proc entry\n");
		return -ENOMEM;
	}
	proc_write_entry->read_proc = read_proc;
	proc_write_entry->write_proc = write_proc;
	return 0;
}

static void __exit hook_exit(void){
	remove_proc_entry("procmon", NULL);
}

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Procmon alternative for Linux");