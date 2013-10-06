#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include "../common/mem_ops.h"
#include "../common/structures.h"
#include "../common/deserialize.h"

void print_info(syscall_info *i);

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024

int sock_fd;
pid_t mypid;
pid_t parentpid;
pid_t parentparentpid;
struct iovec iov;
struct msghdr msg;
struct nlmsghdr *nlh = NULL;
struct sockaddr_nl src_addr, dest_addr;

int main(){

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

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if(sock_fd < 0){
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = mypid;

	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

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

		//Ugly... Only for now...
		if(i->pid != mypid && i->pid != parentpid && i->pid != parentparentpid && strcmp(i->pname, "Xorg") != 0){
			print_info(i);
		}

		del(x);
		del(i);
	}

	close(sock_fd);
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