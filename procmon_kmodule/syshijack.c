#include "syshijack.h"

/*****************************************************************************\
| Pointers to the system call table                                           |
\*****************************************************************************/

void **sys_call_table = NULL;
#ifdef CONFIG_IA32_EMULATION
void **ia32_sys_call_table = NULL;
#endif

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/


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
| Main methods: _init and _exit                                               |
\*****************************************************************************/

static int __init hook_init(void){
	proc_write_entry = proc_create("procmon", 0666, NULL, &proc_file_fops);
	if(proc_write_entry == NULL){
		DEBUG(KERN_INFO "Error creating proc entry\n");
		return -ENOMEM;
	}

	hook_calls();

	return 0;
}

static void __exit hook_exit(void){
	if(is_active()){ //just in case user did not deactivate
		deactivate();
	}
	unhook_calls();

	struct syscall_hash *item, *tmp;
	while(true){
		unsigned long int ncalls = 0;
		HASH_ITER(hh, syscall_items, item, tmp){
			ncalls += item->n_calls;
		}
		if(ncalls == 0){
			HASH_ITER(hh, syscall_items, item, tmp){
				HASH_DEL(syscall_items, item);
				kfree(item);
			}
			break;
		}else{
			msleep_interruptible(500);
		}
	}

	remove_proc_entry("procmon", NULL);
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

module_init(hook_init);
module_exit(hook_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alexander Nestorov <alexandernst@gmail.com>");
MODULE_DESCRIPTION("Procmon alternative for Linux");