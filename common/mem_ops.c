#include "mem_ops.h"

void *new(size_t sz){
#ifdef __KERNEL__
	return kmalloc(sz, GFP_KERNEL);
#else
	return malloc(sz);
#endif
}

void *renew(void *ptr, size_t sz){
#ifdef __KERNEL__
	return krealloc(ptr, sz, GFP_KERNEL);
#else
	return realloc(ptr, sz);
#endif
}

void *duplicate(void *ptr){
#ifdef __KERNEL__
	return kstrdup(ptr, GFP_KERNEL);
#else
	return strdup(ptr);
#endif	
}

void del(void *ptr){
#ifdef __KERNEL__
	kfree(ptr);
#else
	free(ptr);
#endif
}