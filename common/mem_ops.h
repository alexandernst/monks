#ifndef MEM_OPS_H_INCLUDED
#define MEM_OPS_H_INCLUDED

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

void *new(size_t sz);
void *renew(void *ptr, size_t sz);
void *duplicate(void *ptr);
void del(void *ptr);

#endif