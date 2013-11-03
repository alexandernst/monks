#include "lkm.h"

int check(char *kmod_name){
	int ret = 0, err;
	struct kmod_ctx *ctx;
	struct kmod_module *mod;
	struct kmod_list *list, *itr;
	const char *null_config = NULL;

	printf("Checking if %s is loaded...\n", kmod_name);

	ctx = kmod_new(NULL, &null_config);
	if(ctx == NULL){
		ret = -1;
		printf("Unexpected error...\n");
	}else{
		err = kmod_module_new_from_loaded(ctx, &list);
		if(err < 0){
			ret = -1;
			printf("Error: %s\n", strerror(-err));
		}else{
			ret = 0;
			kmod_list_foreach(itr, list){
				mod = kmod_module_get_module(itr);
				if(strcmp(kmod_module_get_name(mod), kmod_name) == 0){
					ret = 1;
				}
				kmod_module_unref(mod);
			}
			kmod_module_unref_list(list);
		}
	}

	kmod_unref(ctx);
	return ret;
}

int load(char *kmod_path){
	int ret = 0, err;
	struct kmod_ctx *ctx;
	struct kmod_module *mod;
	const char *null_config = NULL;

	printf("Loading %s kernel module...\n", kmod_path);

	ctx = kmod_new(NULL, &null_config);
	if(ctx == NULL){
		ret = -1;
		printf("Unexpected error...\n");
	}else{
		err = kmod_module_new_from_path(ctx, kmod_path, &mod);
		if(err != 0){
			ret = -1;
			printf("Error: %s\n", strerror(-err));
		}else{
			err = kmod_module_insert_module(mod, 0, NULL);
			if(err != 0){
				ret = -1;
				printf("Error: %s\n", strerror(-err));
			}else{
				ret = 0;
			}
		}
	}

	kmod_unref(ctx);
	return ret;
}

int unload(char *kmod_name){
	int ret = 0, err;
	struct kmod_ctx *ctx;
	struct kmod_module *mod;
	const char *null_config = NULL;

	printf("Unloading %s kernel module...\n", kmod_name);

	ctx = kmod_new(NULL, &null_config);
	if(ctx == NULL){
		ret = -1;
		printf("Unexpected error...\n");
	}else{
		err = kmod_module_new_from_name(ctx, kmod_name, &mod);
		if(err != 0){
			ret = -1;
			printf("Error: %s\n", strerror(-err));
		}else{
			err = kmod_module_remove_module(mod, 0);
			if(err != 0){
				ret = -1;
				printf("Error: %s\n", strerror(-err));
			}else{
				ret = 0;
			}
		}
	}

	kmod_unref(ctx);
	return ret;
}

/*0 == ok, 1 == ko*/
int start(){
	int ret = 0;
	FILE *fp = fopen("/proc/sys/procmon/state", "w");

	if(fp == NULL){
		ret = 1;
		printf("Error: Can't start hijacking sys calls.\n");
	}else{
		fprintf(fp, "%c", '1');
		fclose(fp);
		ret = 0;
	}

	return ret;
}

/*0 == ok, 1 == ko*/
int stop(){
	int ret = 0;
	FILE *fp = fopen("/proc/sys/procmon/state", "w");

	if(fp == NULL){
		ret = 1;
		printf("Error: Can't start hijacking sys calls.\n");
	}else{
		fprintf(fp, "%c", '0');
		fclose(fp);
		ret = 0;
	}

	return ret;
}