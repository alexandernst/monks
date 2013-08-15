#ifndef SYSCALLS_HASH_H_INCLUDED
#define SYSCALLS_HASH_H_INCLUDED

#include "uthash.h"

/*****************************************************************************\
| Helper macros to keep a hash of hijacked syscalls and how many times are    |
| they called per second                                                      |
\*****************************************************************************/

struct syscall_hash {
	char name[256];
	unsigned long int n_calls;
	UT_hash_handle hh;                                       
};

extern struct syscall_hash *syscall_items;

#define REGISTER(F)                                         \
for(int i = 0; i < 1; i++){                                 \
	struct syscall_hash *syscall =                          \
		(struct syscall_hash*)                              \
		kmalloc(sizeof(struct syscall_hash), GFP_KERNEL);   \
	strncpy(syscall->name, #F, 256);                        \
	syscall->n_calls = 0;                                   \
	HASH_ADD_STR(syscall_items, name, syscall);             \
}

#define INCR_SYSCALL_REG_INFO(F)                            \
struct syscall_hash *item;                                  \
HASH_FIND_STR(syscall_items, #F, item);                     \
if(item){                                                   \
	item->n_calls++;                                        \
}

#define DECR_SYSCALL_REG_INFO(F)                            \
struct syscall_hash *item;                                  \
HASH_FIND_STR(syscall_items, #F, item);                     \
if(item){                                                   \
	item->n_calls--;                                        \
}

#define SHOW_SYSCALL_REG_INFO(F)                            \
struct syscall_hash *item, *tmp;                            \
HASH_ITER(hh, syscall_items, item, tmp){                    \
	printk(KERN_INFO "Name: %s", item->name);               \
	printk(KERN_INFO "N calls/sec: %lu\n", item->n_calls);  \
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif