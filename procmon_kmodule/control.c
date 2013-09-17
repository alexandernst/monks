#include "control.h"
 
/*****************************************************************************\
| /proc/sys vars and methods related to the control of procmon                |
\*****************************************************************************/

int state = 0, min = 0, max = 1;
static struct ctl_table_header *procmon_table_header;

void activate(void){
	state = 1;
}

void deactivate(void){
	state = 0;
}

int is_active(void){
	return state;
}

static ctl_table state_table[] = {
	{
		.procname	= "state",
		.data		= &state,
		.maxlen		= sizeof(int),
		.mode		= 0666,
		.proc_handler	= &proc_dointvec_minmax,
		.extra1		= &min,
		.extra2		= &max
	},
	{ 0 }
};

static ctl_table procmon_table[] = {
	{
		.procname	= "procmon",
		.mode		= 0555,
		.child		= state_table
	},
	{  0 }
};

int register_procmon_sysctl(void){
	procmon_table_header = register_sysctl_table(procmon_table);
	if(!procmon_table_header){
		return -ENOMEM;
	}
	return 0;
}

void unregister_procmon_sysctl(void){
	unregister_sysctl_table(procmon_table_header);
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/