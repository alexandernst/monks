SECTIONS
{
	.syscalls : {
		__start_syscalls = . ;
		*(.syscalls)
		__stop_syscalls = . ;
	}
}