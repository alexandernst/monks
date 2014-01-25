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

typedef enum types {
	NUMBER = 1,
	STRING = 2
} types;

#define PRINT(cur_line, fmt, type, str)                                        \
	{                                                                          \
		int available;                                                         \
		int cur_x, cur_y;                                                      \
		int max_x, max_y;                                                      \
		char *tmp;                                                             \
                                                                               \
		cur_y = max_y; /*var not used*/                                        \
                                                                               \
		getyx(win_data, cur_y, cur_x);                                         \
		getmaxyx(win_data, max_y, max_x);                                      \
                                                                               \
		available = max_x - cur_x;                                             \
		if(available > 0 && cur_y == cur_line){                                \
			tmp = format(fmt, type, str);                                      \
			if(tmp){                                                           \
				waddnstr(win_data, tmp, available);                            \
				del(tmp);                                                      \
			}                                                                  \
		}else{                                                                 \
			continue;                                                          \
		}                                                                      \
	}

char *format(char *fmt, types type, ...);
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