#include "utils.h"

int nl_id = MAX_LINKS - 1;
struct sock *nl_sk = NULL;
extern unsigned int sent_msgs;

static struct sock *nl_init_sock(int netlink_id){
	struct sock * nl_sk;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
	nl_sk = netlink_kernel_create(&init_net, netlink_id, NULL);
#else
	nl_sk = netlink_kernel_create(&init_net, netlink_id, 0, NULL, NULL, THIS_MODULE);
#endif
	return nl_sk;
}

void nl_init(void){
	while (nl_id >= 0) {
		if ((nl_sk = nl_init_sock(nl_id))) {
			DEBUG(KERN_INFO "Acquired NETLINK socket (%d)\n", nl_id);
			return;
		}
		nl_id--;
	}

	DEBUG(KERN_INFO "Error creating socket.\n");
}

void nl_halt(void){
	netlink_kernel_release(nl_sk);
}

void nl_send(syscall_info *i){
	int _client_pid;
	membuffer *x;
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;

	/*If there isn't any client listening, don't try sending anything*/
	_client_pid = get_client_pid();
	if(_client_pid < 0){
		return;
	}

	x = serialize_syscall_info(i);
	if(!x) return;

	skb_out = nlmsg_new(x->len, 0);
	if(!skb_out) goto end;

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, x->len, 0);  
	NETLINK_CB(skb_out).dst_group = 0;
	memcpy(nlmsg_data(nlh), x->data, x->len);
	nlmsg_unicast(nl_sk, skb_out, _client_pid);

	sent_msgs++;

end:
	del(x->data);
	del(x);
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
