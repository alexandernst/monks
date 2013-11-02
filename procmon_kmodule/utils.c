#include "utils.h"

int nl_id = MAX_LINKS - 1;
struct sock *nl_sk = NULL;

/* kernel thread id */
static struct task_struct *nl_thread = NULL;
/* kernel thread waitqueue */
static DECLARE_WAIT_QUEUE_HEAD(nl_wait);
/* kernel thread data queue */
static struct sk_buff_head nl_queue;

static int nl_show_skb(struct sk_buff *skb){
	procmon_info("TODO: show the skb\n");

	return 0;
}

static int nl_send_skb(struct sk_buff *skb){
	int client = get_client_pid();

	/* TODO: make client_pid global and keep it uptodate */

	if(procmon_state && client > 0){
		nlmsg_unicast(nl_sk, skb, client);
	}else{
		if(printk_ratelimit()){
			nl_show_skb(skb);
		}else{
			procmon_warning("printk limit exceeded\n");
		}
		kfree_skb(skb);
	}

	return 0;
}

static int nl_kernel_thread(void *arg){
	DECLARE_WAITQUEUE(wait, current);

	add_wait_queue(&nl_wait, &wait);

	while(!kthread_should_stop()){
		struct sk_buff *skb;

		current->state = TASK_RUNNING;

		while((skb = skb_dequeue(&nl_queue)) != NULL){
			nl_send_skb(skb);
		}

		if(signal_pending(current))
			break;

		current->state = TASK_INTERRUPTIBLE;

		schedule();
	}

	remove_wait_queue(&nl_wait, &wait);

	skb_queue_purge(&nl_queue);

	return 0;
}

static struct sock *nl_init_sock(int netlink_id){
	struct sock *nl_sk;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
	nl_sk = netlink_kernel_create(&init_net, netlink_id, NULL);
#else
	nl_sk = netlink_kernel_create(&init_net, netlink_id, 0, NULL, NULL, THIS_MODULE);
#endif
	return nl_sk;
}

void nl_init(void){
	skb_queue_head_init(&nl_queue);

	nl_thread = kthread_run(nl_kernel_thread, NULL, "kpmnld");
	if (IS_ERR_OR_NULL(nl_thread)) {
		procmon_error("Can't create netlink thread\n");
		return;
	}

	while(nl_id >= 0){
		if((nl_sk = nl_init_sock(nl_id))){
			procmon_info("Acquired NETLINK socket (%d)\n", nl_id);
			return;
		}
		nl_id--;
	}

	kthread_stop(nl_thread), nl_thread = NULL;

	procmon_info("Error creating socket.\n");
}

void nl_halt(void){
	netlink_kernel_release(nl_sk);

	kthread_stop(nl_thread), nl_thread = NULL;
}

void nl_send(syscall_info *i){
	membuffer *x;
	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;

	x = serialize_syscall_info(i);
	if(!x)
		return;

	skb_out = nlmsg_new(x->len, 0);
	if(!skb_out)
		goto end;

	nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, x->len, 0);  
	NETLINK_CB(skb_out).dst_group = 0;
	memcpy(nlmsg_data(nlh), x->data, x->len);

	skb_queue_tail(&nl_queue, skb_out);
	wake_up_interruptible(&nl_wait);

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
