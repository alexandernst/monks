#include "netlink.h"

int get_netlink_id(void){
	FILE *file;
	int netlink_id;

	file = fopen("/proc/sys/procmon/netlink", "r");
	if(file){
		fscanf(file, "%d", &netlink_id);
		fclose(file);
	}

	return netlink_id;
}

int net_init(struct nlmsghdr **nlh, struct msghdr *msg, struct iovec *iov){
	int sock_fd, ret;
	struct sockaddr_nl src_addr;

	sock_fd = socket(PF_NETLINK, SOCK_RAW, get_netlink_id());
	if(sock_fd < 0){
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;

	ret = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
	if(ret != 0){
		return -1;
	}

	*nlh = (struct nlmsghdr *)new(NLMSG_SPACE(MAX_PAYLOAD));
	memset(*nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	(*nlh)->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);

	iov->iov_base = (void *)*nlh;
	iov->iov_len = (*nlh)->nlmsg_len;

	msg->msg_iov = iov;
	msg->msg_iovlen = 1;

	return sock_fd;
}

syscall_info *read_from_socket(int sock_fd, struct nlmsghdr *nlh, struct msghdr msg){
	recvmsg(sock_fd, &msg, 0);

	membuffer *x = new(sizeof(membuffer));
	if(!x){
		return NULL;
	}
	x->len = nlh->nlmsg_len - NLMSG_HDRLEN;
	x->data = new(x->len);
	if(!x->data){
		return NULL;
	}
	memcpy(x->data, NLMSG_DATA(nlh), x->len);

	syscall_info *i = deserialize_syscall_info(x);
	del(x->data);
	del(x);
	if(!i){
		return NULL;
	}

	return i;
}