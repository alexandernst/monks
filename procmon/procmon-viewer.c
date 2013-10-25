#include "procmon-viewer.h"

extern struct iovec iov;
extern struct msghdr msg;
extern struct nlmsghdr *nlh;

unsigned long int total_nodes = 0;
syscall_intercept_info_node *head, *curr, *tail;

int main(int argc, char **argv){

	int sock_fd;
	int ch;
	extern syscall_intercept_info_node *head, *curr;

	while((ch = getopt(argc, argv, "clusevp:")) != -1){
		switch(ch){
			#ifndef __NO_KMOD__

			int ret;

			case 'c':
				ret = check(PROCMON_MODULE_NAME);
				if(ret == 0){
					printf("Procmon kernel module is not loaded.\n");
				}else if(ret == 1){
					printf("Procmon kernel module is loaded.\n");
				}
				return 0;

			case 'l':
				if(load(PROCMON_MODULE_PATH) == 0){
					printf("Procmon kernel module successfully loaded.\n");
				}
				return 0;

			case 'u':
				if(unload(PROCMON_MODULE_NAME) == 0){
					printf("Procmon kernel module successfully unloaded.\n");
				}
				return 0;

			case 's':
				if(start() == 0){
					printf("Procmon kernel module successfully started.\n");
				}
				return 0;

			case 'e':
				if(stop() == 0){
					printf("Procmon kernel module successfully stopped.\n");
				}
				return 0;

			#endif

			case 'v':
				printf("Procmon %g\n", PROCMON_VERSION);
				break;

			case '?':
				printf(
					"Possible options are:\n\t"
						#ifndef __NO_KMOD__
						"'c' - Check if procmon kernel module is loaded.\n\t"
						"'l' - Load procmon kernel module.\n\t"
						"'u' - Unload procmon kernel module.\n\t"
						"'s' - Start procmon kernel module hijack.\n\t"
						"'e' - End procmon kernel module hijack.\n\t"
						#endif
						"'v' - Show procmon version.\n"
				);
				return 1;

			default:
				break;
		}
	}

	sock_fd = net_init();
	if(sock_fd == -1){
		printf("Error starting NetLink.\n");
		return -1;
	}

	/*Make SEGFAULTs play nice with NCURSES*/
	signal(SIGSEGV, do_segfault);

	/*Keep track on the data we have*/
	head = malloc(sizeof(syscall_intercept_info_node));
	head->prev = head->next = NULL;
	head->i = NULL;
	tail = curr = head;

	/*Init ncurses window*/
	initscr();
	noecho();
	cbreak();
	keypad(stdscr, TRUE);
	curs_set(0);
	nodelay(stdscr, 1); //non-blocking getch()

	/*React on resize*/
	signal(SIGWINCH, do_resize);

	/*Get current positiona and size*/
	calc_w_size_pos();
	refresh();
	create_win_data_data_box();

	while((ch = getch()) != 'q'){
		if(recvmsg(sock_fd, &msg, 0) <= 0){
			continue;
		}

		membuffer *x = new(sizeof(membuffer));
		if(!x){
			continue;
		}
		x->len = nlh->nlmsg_len - NLMSG_HDRLEN;
		x->data = new(x->len);
		if(!x->data){
			continue;
		}
		memcpy(x->data, NLMSG_DATA(nlh), x->len);

		syscall_info *i = deserialize_syscall_info(x);
		del(x->data);
		del(x);
		if(!i){
			continue;
		}

		add_data(i);

		if(ch == KEY_UP && curr->prev != NULL){
			curr = curr->prev;
		}else if(ch == KEY_DOWN && curr->next != NULL){
			curr = curr->next;
		}

		draw_data(curr);
	}

	close(sock_fd);

	syscall_intercept_info_node *in = head, *tmp;
	while(!in->next){
		free_info(in->i);

		tmp = in;
		in = in->next;

		del(tmp);
	}

	endwin();
	return 0;
}

void do_segfault(){
	endwin();
	abort();
}

void add_data(syscall_info *i){
	syscall_intercept_info_node *in;

	if(head->i == NULL){
		in = head;
		in->i = i;
	}else{

		if(total_nodes >= MEM_LIMIT){
			syscall_intercept_info_node *tmp = head;
			head = head->next;
			free_info(tmp->i);
			del(tmp);
			total_nodes--;
		}

		in = calloc(sizeof(syscall_intercept_info_node), 1);
		in->prev = tail;
		in->i = i;
		tail->next = in;
		tail = in;
		tail->next = NULL;
	}

	total_nodes++;

	//Auto-scroll!
	if(curr == tail->prev){
		curr = tail;
	}
}

void free_info(syscall_info *i){
	del(i->pname);
	del(i->operation);
	del(i->path);
	del(i->result);
	del(i->details);
	del(i);
}