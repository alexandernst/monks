#include "ui.h"

extern syscall_intercept_info_node *head, *curr;

WINDOW *win_data_box, *win_data;
int win_data_startx, win_data_starty, win_data_width, win_data_height;
int win_data_box_startx, win_data_box_starty, win_data_box_width, win_data_box_height;

char *get_str_info(syscall_info *i){
	int err;
	char *tmp = new(win_data_width + 1);
	if(!tmp){
		return NULL;
	}

	err = snprintf(tmp,
		win_data_width,
		"%-15.20s"       /* Process name */
		"%10u"           /* PID          */
		" %-15.10s"      /* Operation    */
		"%-50.45s"       /* Path         */
		"%-10.10s"       /* Result       */
		" %-200.200s\n", /* Details      */

		i->pname,
		i->pid,
		i->operation,
		i->path,
		i->result,
		i->details
	);

	if(err < 0){
		del(tmp);
		return NULL;
	}

	rstrip(tmp);
	return tmp;
}

void rstrip(char *string){
	int l;
	if(!string){
		return;
	}
	l = strlen(string) - 1;
	while(isspace(string[l]) && l >= 0){
		string[l--] = 0;
	}
}

void lstrip(char *string){
	int i, l;
	if(!string){
		return;
	}
	l = strlen(string);
	while(isspace(string[(i = 0)])){
		while(i++ < l){
			string[i-1] = string[i];
		}
	}
}

void create_win_data_data_box(){
	if(!win_data_box){
		destroy_win(win_data_box);
	}
	if(!win_data){
		destroy_win(win_data);
	}

	refresh();

	win_data_box = create_newwin(win_data_box_height, win_data_box_width, win_data_box_starty, win_data_box_startx);
	win_data = create_newwin(win_data_height, win_data_width, win_data_starty, win_data_startx);

	wclear(win_data);
}

WINDOW *create_newwin(int height, int width, int starty, int startx) {
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);
	wrefresh(local_win);
	return local_win;
}

void destroy_win(WINDOW *local_win) {
	wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(local_win);
	delwin(local_win);
}

void do_resize(){
	endwin();
	refresh();
	clear();

	calc_w_size_pos();
	create_win_data_data_box();

	draw_data(curr);
}

void calc_w_size_pos(){
	win_data_box_height = LINES - 2; //Leave 2 LINES top/bottom margin
	win_data_box_width = COLS - 4; //Leave 4 LINES left/right margin 

	win_data_height = win_data_box_height - 2;
	win_data_width = win_data_box_width - 2;

	win_data_box_starty = (LINES - win_data_box_height) / 2;
	win_data_box_startx = (COLS - win_data_box_width) / 2;

	win_data_starty = win_data_box_starty + 1;
	win_data_startx = win_data_box_startx + 1;
}

void draw_data(syscall_intercept_info_node *in) {
	int i;

	for(i = win_data_height; i >= 0 && in != NULL; i--, in = in->prev){
		if(!filter_i(in->i)){
			char *s_info = get_str_info(in->i);
			if(s_info){
				wclrtoeol(win_data);
				mvwaddstr(win_data, i, 0, s_info);
				del(s_info);
			}else{
				i++;
			}
		}else{
			i++; //Do not leave empty lines!
		}
	}

	wrefresh(win_data);
}

int filter_i(syscall_info *i){
	/*This is temp, will get remove when UI filters start working*/
	pid_t mypid, parentpid;
	mypid = getpid();
	parentpid = getppid();

	if(
		i->pid != mypid && 
		i->pid != parentpid && 
		strcmp(i->pname, "Xorg") != 0 &&
		strcmp(i->pname, "konsole") != 0
	){
		return 0;
	}else{
		return 1;
	}
}