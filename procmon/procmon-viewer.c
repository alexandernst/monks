#include "procmon-viewer.h"

extern struct iovec iov;
extern struct msghdr msg;
extern struct nlmsghdr *nlh;

unsigned long int total_nodes = 0;
syscall_intercept_info_node *head, *curr, *tail;

int main(int argc, char **argv){

	int ch, sock_fd, stdin_fd, efd;
	struct epoll_event event = { .events = 0 }, events[2] = { { .events = 0 }, { .events = 0 } };
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

	/*Get NetLink file descriptor*/
	sock_fd = net_init();
	if(sock_fd == -1){
		printf("Error starting NetLink.\n");
		return -1;
	}
	//fcntl(sock_fd, F_SETFL, fcntl(sock_fd, F_GETFL, 0) | O_NONBLOCK); //Do we need this?

	/*Get STDIN file descriptor*/
	stdin_fd = fcntl(STDIN_FILENO,  F_DUPFD, 0);

	/*Make SEGFAULTs play nice with NCURSES*/
	signal(SIGSEGV, do_segfault);

	/*Create event loop*/
	efd = epoll_create1(0);
	if(efd == -1){
		printf("Error creating epoll\n");
		return -1;
	}

	event.data.fd = sock_fd;
	event.events = EPOLLIN;
	if(epoll_ctl(efd, EPOLL_CTL_ADD, sock_fd, &event) == -1){
		printf("Error adding socket fd to epoll\n");
		return -1;
	}

	event.data.fd = stdin_fd;
	event.events = EPOLLIN;
	if(epoll_ctl(efd, EPOLL_CTL_ADD, stdin_fd, &event) == -1){
		printf("Error adding stdin fd to epoll\n");
		return -1;
	}

	/*Keep track on the data we have*/
	head = new(sizeof(syscall_intercept_info_node));
	if(!head){
		printf("Error allocating memory for data list\n");
		return -1;
	}
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

	/*Start even loop*/
	while(1){
		int n, i, quit = 0;
		n = epoll_wait(efd, events, MAXEVENTS, -1);
		for(i = 0; i < n; i++){
			if(events[i].events & EPOLLIN){
				if(events[i].data.fd == sock_fd){
					read_from_socket(sock_fd);
				}else if(events[i].data.fd == stdin_fd){
					if(read_from_kb() < 0){
						quit = 1;
						break;
					}
				}
			}else if(events[i].events & (EPOLLHUP | EPOLLERR)){
				continue;
			}
		}
		
		if(quit){
			break;
		}
	}

	close(sock_fd);

	/*Free all the memory we allocated for the data*/
	syscall_intercept_info_node *in = head, *tmp;
	while(!in->next){
		free_info(in->i);

		tmp = in;
		in = in->next;

		del(tmp);
	}

	/*Exit*/
	endwin();
	return 0;
}

void do_segfault(){
	endwin();
	abort();
}

int read_from_kb(void){
	int ch = getch();

	//We need to assign curr the next *visible* element
	//aka, the one that will pass the filter options
	if(ch == KEY_UP && curr->prev != NULL){
		curr = curr->prev;
		while(!filter_i(curr->i)){
			curr = curr->prev;
			if(curr == head){
				break;
			}
		}
	}else if(ch == KEY_DOWN && curr->next != NULL){
		curr = curr->next;
		while(!filter_i(curr->i)){
			curr = curr->next;
			if(curr == tail){
				break;
			}
		}
	}else if(ch == 'q'){
		return -1;
	}

	draw_data(curr);
	return 0;
}

void read_from_socket(int sock_fd){
	recvmsg(sock_fd, &msg, 0);

	membuffer *x = new(sizeof(membuffer));
	if(!x){
		return;
	}
	x->len = nlh->nlmsg_len - NLMSG_HDRLEN;
	x->data = new(x->len);
	if(!x->data){
		return;
	}
	memcpy(x->data, NLMSG_DATA(nlh), x->len);

	syscall_info *i = deserialize_syscall_info(x);
	del(x->data);
	del(x);
	if(!i){
		return;
	}

	add_data(i);

	//Don't draw if there's nothing new to draw
	if(curr == tail){
		draw_data(curr);
	}
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