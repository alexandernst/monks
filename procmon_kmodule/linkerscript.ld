SECTIONS
{
	.counters : {
		__start_counters = . ;
		*(.counters)
		__stop_counters = . ;
	}
}