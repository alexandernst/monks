#include "procmon-viewer.h"

int sock_fd;
struct iovec iov;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;

int main(int argc, char **argv){

	int ch, pid_filter = -1;
	pid_t mypid, parentpid, parentparentpid;
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

			case 'p':
				pid_filter = atoi(optarg);
				printf("Including process %d\n", pid_filter);
				break;

			case 'v':
				printf("Procmon %g\n", PROCMON_VERSION);
				break;

			case '?':
				if(optopt == 'p'){
					fprintf(stderr, "Option -%c requires an argument (PID).\n", optopt);
				}else{
					printf(
						"Possible options are:\n\t"
							#ifndef __NO_KMOD__
							"'c' - Check if procmon kernel module is loaded.\n\t"
							"'l' - Load procmon kernel module.\n\t"
							"'u' - Unload procmon kernel module.\n\t"
							"'s' - Start procmon kernel module hijack.\n\t"
							"'e' - End procmon kernel module hijack.\n\t"
							#endif
							"'v' - Show procmon version.\n\t"
							"'p' <PID> - Exclude PID from returned results. (This option is temporal until UI is working properly)\n"
					);
				}
				return 1;

			default:
				break;
		}
	}

	if(net_init() == -1){
		printf("Error starting NetLink.\n");
		return -1;
	}

	/*Make SEGFAULTs play nice with NCURSES*/
	signal(SIGSEGV, do_segfault);

	/*Keep track on the data we have*/
	head = malloc(sizeof(syscall_intercept_info_node));
	head->prev = head->next = NULL;
	head->i = NULL;
	curr = head;

	/*This is temp, will get remove when UI filters start working*/
	mypid = getpid();
	parentpid = getppid();

	/*Ugly... Only for now... Get the PID of the parent of our parent*/
	char procpath[256];
	FILE *procstat;

	snprintf(procpath, 256, "/proc/%u/stat", parentpid);
	procstat = fopen(procpath, "r");
	if(!procstat){
		return 0;
	}
	fscanf(procstat, "%*d %*s %*c %u", &parentparentpid);
	fclose(procstat);

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
		recvmsg(sock_fd, &msg, 0);

		char *q = (char *)NLMSG_DATA(nlh);
		membuffer *x = deserialize_membuffer(q);
		if(!x){
			continue;
		}

		syscall_info *i = deserialize_syscall_info(x);
		del(x);
		if(!i){
			continue;
		}

		//Ugly... Only for now...
		if(	i->pid != mypid && 
			i->pid != parentpid && 
			i->pid != parentparentpid &&
			i->pid != pid_filter &&
			strcmp(i->pname, "Xorg") != 0)
		{
			add_data(i);
		}

		if(ch == KEY_UP && curr->prev != NULL){
			curr = curr->prev;
		}else if(ch == KEY_DOWN && curr->next != NULL){
			curr = curr->next;
		}

		draw_data(curr);

		//del(i);
	}

	close(sock_fd);

	endwin();
	return 0;
}

void do_segfault(){
	endwin();
	abort();
}

int get_netlink_id(void)
{
	FILE * file;
	int nl_id = MAX_LINKS - 1;

	file = fopen("/proc/sys/procmon/netlink", "r");
	if (file) {
		fscanf(file, "%d", &nl_id);
		fclose(file);
	}

	return nl_id;
}

int net_init(){
	struct sockaddr_nl src_addr, dest_addr;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, get_netlink_id());
	if(sock_fd < 0){
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;
	dest_addr.nl_groups = 0;

	nlh = (struct nlmsghdr *)new(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	strcpy(NLMSG_DATA(nlh), "");

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	sendmsg(sock_fd, &msg, 0);

	return 0;
}

