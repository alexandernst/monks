#ifndef UI_H_INCLUDED
#define UI_H_INCLUDED

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>

#include "../common/mem_ops.h"
#include "../common/structures.h"

char *get_str_info(syscall_intercept_info *i);
void rstrip(char *string);
void lstrip(char *string);

void init_ncurses(void);
void create_win_data_data_box();
WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

void schedule_resize();
void calc_w_size_pos();

void draw_data(syscall_intercept_info_node *l);
int filter_i(syscall_intercept_info *i);
int read_from_kb(void);

#endif