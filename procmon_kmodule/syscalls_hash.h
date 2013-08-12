#ifndef SYSCALLS_HASH_H_INCLUDED
#define SYSCALLS_HASH_H_INCLUDED

#include "uthash.h"

/*****************************************************************************\
| Helper macros to keep a hash of hijacked syscalls and how many times are    |
| they called per second                                                      |
\*****************************************************************************/

struct syscall_hash {
	char name[256];
	unsigned long int calls_in_last_sec;
	UT_hash_handle hh;                                       
};

extern struct syscall_hash *syscall_items;

#define REGISTER(F)                                         \
struct syscall_hash *syscall =                              \
	(struct syscall_hash*)                                  \
	kmalloc(sizeof(struct syscall_hash), GFP_KERNEL);       \
strncpy(syscall->name, #F, 256);                            \
syscall->calls_in_last_sec = 0;                             \
HASH_ADD_STR(syscall_items, name, syscall);  

#define INCR_SYSCALL_REG_INFO(F)                            \
struct syscall_hash *item;                                  \
HASH_FIND_STR(syscall_items, #F, item);                     \
if(item){                                                   \
	if(item->calls_in_last_sec == ULONG_MAX){               \
		item->calls_in_last_sec = 1;                        \
	}else{                                                  \
		item->calls_in_last_sec++;                          \
	}                                                       \
}

#define SHOW_SYSCALL_REG_INFO(F)                            \
struct syscall_hash *item, *tmp;                            \
HASH_ITER(hh, syscall_items, item, tmp){                    \
	printk(KERN_INFO "Name: %s", item->name);               \
	printk(KERN_INFO "N calls/sec: %lu\n",                  \
		item->calls_in_last_sec);                           \
}

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif