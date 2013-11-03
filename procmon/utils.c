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