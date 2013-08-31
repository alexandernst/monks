#ifndef CONTROL_RO_RW_SYSCALL_TABLE_H_INCLUDED
#define CONTROL_RO_RW_SYSCALL_TABLE_H_INCLUDED

#include "syshijack.h"

int make_rw(unsigned long address);
int make_ro(unsigned long address);

void setback_cr0(unsigned long val);
unsigned long clear_and_return_cr0(void);

int get_sct(void);
int set_sct_rw(void);
int set_sct_ro(void);

#endif