#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#else
#include <stdlib.h>
#include <stdio.h>
#endif

void *new(size_t sz);
void *renew(void *ptr, size_t sz);
void del(void *ptr);
