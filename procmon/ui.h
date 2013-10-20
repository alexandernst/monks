#ifndef UI_H
#define UI_H

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "../common/mem_ops.h"
#include "../common/structures.h"

char *get_str_info(syscall_info *i);
void rstrip(char *string);
void lstrip(char *string);

void create_win_data_data_box();
WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

void do_resize();
void calc_w_size_pos();

void add_data(syscall_info *i);
void draw_data(syscall_intercept_info_node *l); 

#endif