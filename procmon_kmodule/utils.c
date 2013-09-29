#include "utils.h"

#define NETLINK_USER 31
int client_pid = 0;
struct sock *nl_sk = NULL;

void nl_init(void){
	/* This is for 3.6 kernels and above.*/
	struct netlink_kernel_cfg cfg = {
		.input = nl_recv,
	};

	nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
	/*Bellow kernel 3.6*/
	//nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, 0, nl_recv, NULL, THIS_MODULE);
	
	if(!nl_sk){
		DEBUG(KERN_INFO "Error creating socket.\n");
	}
}

void nl_halt(void){
	netlink_kernel_release(nl_sk);
}

void nl_recv(struct sk_buff *skb){
	struct nlmsghdr *nlh;

	nlh = (struct nlmsghdr*)skb->data;
	client_pid = nlh->nlmsg_pid;
}

void nl_send(syscall_info *i){
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	int msg_size;

	membuffer *x = serialize_syscall_info(i);
	byte *data = x->data;

	char *q = serialize_membuffer(x);
	msg_size = sizeof(size_t) + x->len;

	skb_out = nlmsg_new(msg_size, 0);

	if(!skb_out){
		return;
	} 
	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);  
	NETLINK_CB(skb_out).dst_group = 0;
	memcpy(nlmsg_data(nlh), q, msg_size);

	nlmsg_unicast(nl_sk, skb_out, client_pid);

	del(data);
	del(x);
	del(q);
}

char *path_from_fd(unsigned int fd){
	char *tmp;
	char *pathname = "";
	char *rpathname = new(1);

	struct file *file;
	struct path path;
	struct files_struct *files = current->files;

	spin_lock(&files->file_lock);
	file = fcheck_files(files, fd);
	if (!file) {
		spin_unlock(&files->file_lock);
		return rpathname;
	}

	path = file->f_path;
	path_get(&file->f_path);
	spin_unlock(&files->file_lock);
	
	tmp = (char *)__get_free_page(GFP_TEMPORARY);
	if(!tmp){
		path_put(&path);
		return rpathname;
	}

	pathname = d_path(&path, tmp, PAGE_SIZE);
	path_put(&path);

	if(IS_ERR(pathname)){
		free_page((unsigned long)tmp);
		return rpathname;
	}

	rpathname = kstrdup(pathname, GFP_KERNEL);
	
	free_page((unsigned long)tmp);
	
	return rpathname;
}
