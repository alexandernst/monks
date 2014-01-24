#ifndef MSGS_H_INCLUDED
#define MSGS_H_INCLUDED

#define monks_msg(level, fmt...)                      \
	printk(level "[" KBUILD_MODNAME "]" " " fmt)

#define monks_info(fmt...)                            \
	monks_msg(KERN_INFO, fmt)
#define monks_warning(fmt...)                         \
	monks_msg(KERN_WARNING, fmt)
#define monks_error(fmt...)                           \
	monks_msg(KERN_ERR, fmt)

#endif