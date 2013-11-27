#include "netlink.h"

int net_init(struct nlmsghdr **nlh, struct iovec *iov){
	int sock_fd, ret;
	struct sockaddr_nl src_addr;

	sock_fd = socket(AF_NETLINK, SOCK_DGRAM, get_netlink_id());
	if(sock_fd < 0){
		return -1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();

	ret = bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));
	if(ret != 0){
		return -1;
	}

	*nlh = (struct nlmsghdr *)new(NLMSG_SPACE(MAX_PAYLOAD));
	memset(*nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
	(*nlh)->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);

	iov->iov_base = (void *)*nlh;
	iov->iov_len = (*nlh)->nlmsg_len;

	return sock_fd;
}

syscall_intercept_info *read_from_socket(int sock_fd, struct nlmsghdr *nlh){
	membuffer *x;
	syscall_intercept_info *i;
	struct iovec iov;
	struct msghdr msg;

	iov.iov_base = nlh;
	iov.iov_len = NLMSG_SPACE(MAX_PAYLOAD);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = 0;

	if (recvmsg(sock_fd, &msg, MSG_DONTWAIT) <= 0)
		return NULL;

	x = new(sizeof(membuffer));
	if(!x){
		return NULL;
	}
	x->len = nlh->nlmsg_len - NLMSG_HDRLEN;
	x->data = new(x->len);
	if(!x->data){
		return NULL;
	}
	memcpy(x->data, NLMSG_DATA(nlh), x->len);

	i = deserialize_syscall_info(x);
	del(x->data);
	del(x);
	if(!i){
		return NULL;
	}

	return i;
}