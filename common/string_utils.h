#ifndef STRING_UTILS_H_INCLUDED
#define STRING_UTILS_H_INCLUDED

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>

#include "mem_ops.h"
#include "structures.h"

typedef enum types {
	NUMBER = 1,
	STRING = 2
} types;

char *format(char *fmt, types type, ...);
char *get_str_info(syscall_intercept_info *i);
void rstrip(char *string);
void lstrip(char *string);

#endif