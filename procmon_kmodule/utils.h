#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "main.h"

/*****************************************************************************\
| Utils                                                                       |
\*****************************************************************************/

typedef struct syscall_intercept_info{
	char *pname;
	pid_t pid;
	char *operation;
	char *path;
	char *result;
	char *details;
} syscall_info;

void print_info(syscall_info *i);
char *path_from_fd(unsigned int fd);

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif