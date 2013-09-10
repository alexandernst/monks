#include "hookfns.h"

/*****************************************************************************\
| This is where the magic happens. We call HOOK (and maybe HOOK_IA32) for     |
| each syscall.                                                               |
| The macros HOOK and HOOK_IA32 replace the REAL functions with the FAKE      |
| ones. See syshijack.h for more info.                                        |
\*****************************************************************************/

asm(".section counters, \"aw\""); //set section allocatable and writable

void hook_calls(void){
	if(get_sct() && set_sct_rw()){

/* __NR_read / __NR32_read */
		HOOK(read);
#ifdef __NR32_read
		HOOK_IA32(read);
#endif

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
	if(get_sct() && set_sct_rw()){

/* __NR_read / __NR_read32 */
		UNHOOK(read);
#ifdef __NR32_read
		UNHOOK_IA32(read);
#endif

		set_sct_ro();
	}
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/