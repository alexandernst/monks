#include "utils.h"

void set_client_pid(int pid){
	FILE *file;

	/*Set our client PID in Procmon*/
	file = fopen("/proc/sys/procmon/client_pid", "w");
	if(file){
		fprintf(file, "%d", pid);
		fclose(file);
	}
}

int get_netlink_id(void){
	FILE *file;
	int netlink_id;

	/*Get the NetLink ID of Procmon*/
	file = fopen("/proc/sys/procmon/netlink", "r");
	if(file){
		fscanf(file, "%d", &netlink_id);
		fclose(file);
	}else{
		netlink_id = -1;
	}

	return netlink_id;
}