#ifndef STRUCTURES_H_INCLUDED
#define STRUCTURES_H_INCLUDED

typedef unsigned char byte;

typedef struct membuffer{
	size_t len;
	byte *data;
} membuffer;

typedef struct syscall_intercept_info{
	char *pname;
	pid_t pid;
	char *operation;
	char *path;
	char *result;
	char *details;
} syscall_info;

#endif