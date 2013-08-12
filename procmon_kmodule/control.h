#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

/*****************************************************************************\
| Control                                                                     |
\*****************************************************************************/

extern char proc_data[1];
extern struct proc_dir_entry *proc_write_entry;
extern const struct file_operations proc_file_fops;

void activate(void);
void deactivate(void);
int is_active(void);
ssize_t read_proc(struct file *file, char __user *buf, size_t count, loff_t *pos);
ssize_t write_proc(struct file *file, const char __user *buf, size_t count, loff_t *pos);

/*****************************************************************************\
|                                      END                                    |
\*****************************************************************************/

#endif