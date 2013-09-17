#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#include "syshijack.h"

/*****************************************************************************\
| Control                                                                     |
\*****************************************************************************/

void activate(void);
void deactivate(void);
int is_active(void);

int register_procmon_sysctl(void);
void unregister_procmon_sysctl(void);

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif