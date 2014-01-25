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

typedef enum colors {
	GREEN = 1,
	RED = 2,
	YELLOW = 3
} colors;

/*****************************************************************************\
| SET_ON_COLOR is a utils function that sets a color (pair) after ebaluating  |
| a condition. It takes the following arguments:                              |
|                                                                             |
| window - which window to apply the color to                                 |
| result_color - will be set to the color that was chosen after evaluating    |
|                the condition                                                |
| color_true - color (pair) which will be set if the condition is true        |
| color_false - color (pair) which will be set if the condition is false.     |
|               This value can take a 0 as value, in which case no color      |
|               will be set.                                                  |
| condition - the condition which should be evaluated                         |
|                                                                             |
\*****************************************************************************/

#define SET_ON_COLOR(window, result_color, color_true, color_false, condition)\
	if(has_colors()){                                                         \
		if(condition){                                                        \
			result_color = color_true;                                        \
		}else{                                                                \
			result_color = color_false;                                       \
		}                                                                     \
		if(result_color){                                                     \
			wattron(window, COLOR_PAIR(result_color));                        \
		}                                                                     \
	}

/*****************************************************************************\
| SET_OFF_COLOR is the reverse of SET_ON_COLOR. It will turn off the color    |
| that was set by SET_ON_COLOR. SET_OFF_COLOR accepts two arguments:          |
|                                                                             |
| window - which window to apply the color to                                 |
| result_color - the color that was set by SET_ON_COLOR.                      |
\*****************************************************************************/

#define SET_OFF_COLOR(window, result_color)                                   \
	if(result_color && has_colors()){                                         \
		wattroff(window, COLOR_PAIR(result_color));                           \
	}

#define PRINT(window, cur_line, fmt, type, str)                               \
	{                                                                         \
		int available;                                                        \
		int cur_x, cur_y;                                                     \
		int max_x, max_y;                                                     \
		char *tmp;                                                            \
                                                                              \
		cur_y = max_y; /*var not used*/                                       \
                                                                              \
		getyx(window, cur_y, cur_x);                                          \
		getmaxyx(window, max_y, max_x);                                       \
                                                                              \
		available = max_x - cur_x;                                            \
		if(available > 0 && cur_y == cur_line){                               \
			tmp = format(fmt, type, str);                                     \
			if(tmp){                                                          \
				waddnstr(window, tmp, available);                             \
				del(tmp);                                                     \
			}                                                                 \
		}else{                                                                \
			continue;                                                         \
		}                                                                     \
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