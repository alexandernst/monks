#include "ui.h"

static int scheduled_resize = 0;
extern syscall_intercept_info_node *head, *curr, *tail;

WINDOW *header_box, *win_data_box, *win_data;
int header_box_startx, header_box_starty, header_box_width, header_box_height;
int win_data_startx, win_data_starty, win_data_width, win_data_height;
int win_data_box_startx, win_data_box_starty, win_data_box_width, win_data_box_height;

char *format(char *fmt, types type, ...){
	int err;
	va_list ap;

	char *tmp = new(win_data_width + 1);
	if(!tmp){
		return NULL;
	}

	va_start(ap, type);

	if(type == NUMBER){
		int val = va_arg(ap, int);
		err = snprintf(tmp, win_data_width, fmt, val);
	}else if(type == STRING){
		char *val = va_arg(ap, char*);
		err = snprintf(tmp, win_data_width, fmt, val);
	}

	va_end(ap);

	if(err < 0){
		del(tmp);
		return NULL;
	}

	return tmp;
}

char *get_str_info(syscall_intercept_info *i){
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

void init_ncurses(void){
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);
	nodelay(stdscr, 1); /*non-blocking getch()*/
}

void create_win_data_data_box(){
	if(!win_data_box){
		destroy_win(win_data_box);
	}
	if(!win_data){
		destroy_win(win_data);
	}

	refresh();

	header_box = create_newwin(header_box_height, header_box_width, header_box_starty, header_box_startx);
	mvwprintw(header_box, 1, 1,
		"%-15.20s"       /* Process name */
		"%10s"           /* PID          */
		" %-15.10s"      /* Operation    */
		"%-50.45s"       /* Path         */
		"%-10.10s"       /* Result       */
		" %-7.7s",       /* Details      */

		"Process name", "PID", "Operation", "Path", "Result", "Details");
	wrefresh(header_box);

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

void schedule_resize(){
	scheduled_resize = 1;
}

void calc_w_size_pos(){
	/*External box (frame)*/
	win_data_box_height = LINES - 4; /*Leave 2 LINES top/bottom margin*/
	win_data_box_width = COLS - 4; /*/Leave 4 LINES left/right margin */

	win_data_box_starty = (LINES - win_data_box_height) / 2 + 1;
	win_data_box_startx = (COLS - win_data_box_width) / 2;

	/*Data box*/
	win_data_height = win_data_box_height - 2;
	win_data_width = win_data_box_width - 2;

	win_data_starty = win_data_box_starty + 1;
	win_data_startx = win_data_box_startx + 1;

	/*Header box*/
	header_box_height = 3;
	header_box_width = win_data_box_width;

	header_box_starty = win_data_box_starty - 3;
	header_box_startx = win_data_box_startx;
}

void draw_data(syscall_intercept_info_node *in) {
	int i;

	if(scheduled_resize){
		endwin();
		refresh();
		clear();

		calc_w_size_pos();
		create_win_data_data_box();

		scheduled_resize = 0;
	}

	for(i = win_data_height; i >= 0 && in != NULL && in != head; i--, in = in->prev){

		/*Draw or skip data node but keep cursor on the same line*/
		if(!filter_i(in->i)){

			/*Clear line before drawing on it*/
			wmove(win_data, i, 0);
			wclrtoeol(win_data);


			PRINT(i, "%-15.20s", STRING, in->i->pname);

			PRINT(i, "%10u", NUMBER, in->i->pid);

			PRINT(i, " %-15.10s", STRING, in->i->operation);

			PRINT(i, "%-50.45s", STRING, in->i->path);

			PRINT(i, "%-10.10s", STRING, in->i->result);

			PRINT(i, " %-200.200s", STRING, in->i->details);

		}else{
			i++; /*Do not leave empty lines!*/
		}
	}

	wrefresh(win_data);
}

int filter_i(syscall_intercept_info *i){
	/*This is temp, will get remove when UI filters start working*/
	pid_t mypid, parentpid;
	mypid = getpid();
	parentpid = getppid();

	if(
		i->pid != mypid && 
		i->pid != parentpid && 
		strcmp(i->pname, "X") != 0 &&
		strcmp(i->pname, "Xorg") != 0 &&
		strcmp(i->pname, "konsole") != 0
	){
		return 0;
	}else{
		return 1;
	}
}

int read_from_kb(void){
	int ch = getch();

	/*We need to assign curr the next *visible* element
	(the one that will pass the filter options)*/
	if(ch == KEY_UP && curr->prev){
		curr = curr->prev;
		while(filter_i(curr->i)){
			curr = curr->prev;
			if(curr == head){
				break;
			}
		}
	}else if(ch == KEY_DOWN && curr->next){
		curr = curr->next;
		while(filter_i(curr->i)){
			curr = curr->next;
			if(curr == tail){
				break;
			}
		}
	}else if(ch == 'q'){
		return -1;
	}

	return 0;
}