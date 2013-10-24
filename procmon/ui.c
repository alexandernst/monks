#include "ui.h"

WINDOW *win_data_box, *win_data;
syscall_intercept_info_node *head, *curr;
int win_data_startx, win_data_starty, win_data_width, win_data_height;
int win_data_box_startx, win_data_box_starty, win_data_box_width, win_data_box_height;

char *get_str_info(syscall_info *i){
	char *tmp = new(win_data_width);
	snprintf(tmp,
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

void add_data(syscall_info *i) {
	syscall_intercept_info_node *in;

	if(head->i == NULL){
		in = head;
		in->prev = NULL;
		in->next = NULL;
		in->i = i;
		curr = in;
	}else{
		in = calloc(sizeof(syscall_intercept_info_node), 1);
		in->prev = curr;
		in->i = i;
		curr->next = in;
		curr = in;
		curr->next = NULL;
	}

}

void draw_data(syscall_intercept_info_node *in) {
	int i;

	/*This is temp, will get remove when UI filters start working*/
	pid_t mypid, parentpid;
	mypid = getpid();
	parentpid = getppid();

	for(i = win_data_height; i >= 0 && in != NULL; i--, in = in->prev){
		if(	in->i != NULL && //Will be there in->i == NULL ever?

			//Ugly... Only for now...
			in->i->pid != mypid && 
			in->i->pid != parentpid && 
			strcmp(in->i->pname, "Xorg") != 0 &&
			strcmp(in->i->pname, "konsole") != 0
		){
			char *s_info = get_str_info(in->i);
			wclrtoeol(win_data);
			mvwaddstr(win_data, i, 0, s_info);
			del(s_info);
		}else{
			i++; //Do we really need this?
		}
	}

	wrefresh(win_data);
}