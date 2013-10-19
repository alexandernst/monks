#include "procmon-viewer.h"

int sock_fd;
pid_t mypid;
pid_t parentpid;
pid_t parentparentpid;
struct iovec iov;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;
struct sockaddr_nl src_addr, dest_addr;

int main(int argc, char *argv[]){

	int c;
	char *pvalue = NULL;
	while((c = getopt(argc, argv, "clusevp:")) != -1){
		switch(c){
			#ifndef __NO_KMOD__

			int ret;

			case 'c':
				ret = check(PROCMON_MODULE_PATH);
				if(ret == 0){
					printf("Procmon kernel module is not loaded.\n");
				}else if(ret == 1){
					printf("Procmon kernel module is loaded.\n");
				}
				break;

			case 'l':
				if(load(PROCMON_MODULE_PATH) == 0){
					printf("Procmon kernel module successfully loaded.\n");
				}
				break;

			case 'u':
				if(unload(PROCMON_MODULE_PATH) == 0){
					printf("Procmon kernel module successfully unloaded.\n");
				}
				break;

			case 's':
				if(start() == 0){
					printf("Procmon kernel module successfully started.\n");
				}
				break;

			case 'e':
				if(stop() == 0){
					printf("Procmon kernel module successfully stopped.\n");
				}
				break;

			#endif

			case 'p':
				pvalue = optarg;
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

	int proc_num = -1;

	if(pvalue){
		proc_num = atoi(pvalue);
		printf("Including process %d\n", proc_num);
	}

	mypid = getpid();
	parentpid = getppid();

	//Ugly... Only for now...
	pid_t ppid;
	char procpath[256];
	FILE *procstat;

	snprintf(procpath, 256, "/proc/%u/stat", parentpid);
	procstat = fopen(procpath, "r");
	if(!procstat) {
		return 0;
	}
	fscanf(procstat, "%*d %*s %*c %u", &ppid);
	fclose(procstat);
	parentparentpid = ppid;

	if(net_init() == -1){
		return -1;
	}

	while(1){
		recvmsg(sock_fd, &msg, 0);

		char *q = (char *)NLMSG_DATA(nlh);
		membuffer *x = deserialize_membuffer(q);
		if(!x){
			continue;
		}

		syscall_info *i = deserialize_syscall_info(x);
		if(!i){
			del(x);
			continue;
		}

		if(proc_num > 0 && i->pid != proc_num){
			continue;
		}

		//Ugly... Only for now...
		if(i->pid != mypid && i->pid != parentpid && i->pid != parentparentpid && strcmp(i->pname, "Xorg") != 0){
			print_info(i);
		}

		del(x);
		del(i);
	}

	close(sock_fd);
}

int net_init(){
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if(sock_fd < 0){
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = mypid;

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

void print_info(syscall_info *i){
	printf(
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
}
