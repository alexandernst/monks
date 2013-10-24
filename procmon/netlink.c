#include "netlink.h"

struct iovec iov;
struct msghdr msg;
struct nlmsghdr *nlh;

int get_netlink_id(void){
	FILE * file;
	int nl_id = MAX_LINKS - 1;

	file = fopen("/proc/sys/procmon/netlink", "r");
	if(file){
		fscanf(file, "%d", &nl_id);
		fclose(file);
	}

	return nl_id;
}

int net_init(void){
	int sock_fd;
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

	return sock_fd;
}