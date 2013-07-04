#include "syshijack.h"

//TODO
//GLOBAL HOOK with format:
//Time				Process name	PID		Operation						Path																		Result		Detail

//19:25:37,7981199	lsass.exe		696		RegOpenKey						HKLM\SECURITY\Policy														SUCCESS		Desired Access: Read/Write
//19:25:53,7759095	svchost.exe		1116	CreateFile						C:\WINDOWS\system32\drivers\msfs.sys										SUCCESS		Desired Access: Read EA, Read Attributes, Read Control, Disposition: Open, Options: , Attributes: n/a, ShareMode: Read, Write, Delete, AllocationSize: n/a, Impersonating: NT AUTHORITY\SYSTEM, OpenResult: Opened
//19:25:55,1153041	wmiprvse.exe	3292	RegQueryValue					HKLM\SYSTEM\WPA\MediaCenter\Installed										SUCCESS		Type: REG_DWORD, Length: 4, Data: 0
//19:26:02,6182319	SDFSSvc.exe		288		QueryStandardInformationFile	C:\Archivos de programa\Spybot - Search & Destroy 2\Includes\HostScan.csbs1	SUCCESS		AllocationSize: 0, EndOfFile: 0, NumberOfLinks: 1, DeletePending: False, Directory: False


asmlinkage long (*real_sys_read)(unsigned int fd, char __user *buf, size_t count);
asmlinkage long hooked_sys_read(unsigned int fd, char __user *buf, size_t count){
	if(count == 1 && fd == 0)
		printk(KERN_INFO "intercepted sys_read from %s\n", current->comm);
	return real_sys_read(fd, buf, count);
}

void hook_calls(void){
	sys_call_table = get_writable_sct(get_sys_call_table());
	if(sys_call_table == NULL){
		printk(KERN_INFO "sys_call_table is NULL\n");
		return;
	}
#ifdef CONFIG_IA32_EMULATION
	ia32_sys_call_table = get_writable_sct(get_ia32_sys_call_table());
	if(ia32_sys_call_table == NULL){
		vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
		printk(KERN_INFO "ia32_sys_call_table is NULL\n");
		return;
	}
#endif



	HOOK(__NR_read, real_sys_read, hooked_sys_read);
#ifdef __NR_read32
	HOOK_IA32(__NR_read32, real_sys_read, hooked_sys_read);
#endif



}

void unhook_calls(void){



	UNHOOK(__NR_read, real_sys_read);
#ifdef __NR_read32
	UNHOOK_IA32(__NR_read32, real_sys_read);
#endif



	vunmap((void*)((unsigned long)sys_call_table & PAGE_MASK));
#ifdef CONFIG_IA32_EMULATION
	vunmap((void*)((unsigned long)ia32_sys_call_table & PAGE_MASK));
#endif
}