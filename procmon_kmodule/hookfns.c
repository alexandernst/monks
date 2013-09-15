#include "hookfns.h"

/*****************************************************************************\
| This is where the magic happens. We call HOOK (and maybe HOOK_IA32) for     |
| each syscall.                                                               |
| The macros HOOK and HOOK_IA32 replace the REAL functions with the FAKE      |
| ones. See syshijack.h for more info.                                        |
\*****************************************************************************/

asm(".section .counters, \"aw\""); //set section allocatable and writable

void hook_calls(void){

	counter_info_t *iter;

	if(get_sct() && set_sct_rw()){

		iter = __start_counters;
		for(; iter < __stop_counters; ++iter){
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

		iter = __start_counters;
		for(; iter < __stop_counters; ++iter){
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