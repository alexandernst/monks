#ifndef MSGS_H_INCLUDED
#define MSGS_H_INCLUDED

#define procmon_msg(level, fmt...)					\
	printk(level "[" KBUILD_MODNAME "]" " " fmt)

#define procmon_info(fmt...)						\
	procmon_msg(KERN_INFO, fmt)
#define procmon_warning(fmt...)						\
	procmon_msg(KERN_WARNING, fmt)
#define procmon_error(fmt...)						\
	procmon_msg(KERN_ERR, fmt)

#endif