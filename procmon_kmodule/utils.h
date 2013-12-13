#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/fdtable.h>

#include "../common/mem_ops.h"

/*****************************************************************************\
| Utils                                                                       |
\*****************************************************************************/

void *map_writable(void *addr, size_t len);

char *path_from_fd(unsigned int fd);

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif
