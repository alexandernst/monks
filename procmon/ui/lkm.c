#include "ruby.h"
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <libkmod.h>

VALUE lkm = Qnil;

void Init_lkm();
static VALUE check(VALUE self, VALUE kmod_name);
static VALUE load(VALUE self, VALUE kmod_path);
static VALUE unload(VALUE self, VALUE kmod_name);
static VALUE start(VALUE self);
static VALUE stop(VALUE self);

void Init_lkm() {
	lkm = rb_define_module("LKM");
	rb_define_method(lkm, "check", check, 1);
	rb_define_method(lkm, "load", load, 1);
	rb_define_method(lkm, "unload", unload, 1);
	rb_define_method(lkm, "start", start, 0);
	rb_define_method(lkm, "stop", stop, 0);
}

static VALUE check(VALUE self, VALUE kmod_name){
	printf("Checking if %s is loaded...\n", RSTRING_PTR(kmod_name));

	int ret = 0, err;
	struct kmod_ctx *ctx;
	struct kmod_module *mod;
	struct kmod_list *list, *itr;
	const char *null_config = NULL;
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
				if(strcmp(kmod_module_get_name(mod), RSTRING_PTR(kmod_name)) == 0)
					ret = 1;
				kmod_module_unref(mod);
			}
			kmod_module_unref_list(list);
		}
	}

	kmod_unref(ctx);
	return INT2NUM(ret);
}

static VALUE load(VALUE self, VALUE kmod_path){
	printf("Loading %s kernel module...\n", RSTRING_PTR(kmod_path));

	int ret = 0, err;
	struct kmod_ctx *ctx;
	struct kmod_module *mod;
	struct kmod_list *list, *itr;
	const char *null_config = NULL;

	ctx = kmod_new(NULL, &null_config);
	if(ctx == NULL){
		ret = -1;
		printf("Unexpected error...\n");
	}else{
		err = kmod_module_new_from_path(ctx, RSTRING_PTR(kmod_path), &mod);
		if(err != 0){
			ret = -1;
			printf("Error: %s\n", strerror(-err));
		}else{
			err = kmod_module_insert_module(mod, 0, NULL);
			if(err != 0){
				ret = -1;
				printf("Error 1: %s\n", strerror(-err));
			}else{
				ret = 0;
			}
		}
	}

	kmod_unref(ctx);
	return INT2NUM(ret);
}

static VALUE unload(VALUE self, VALUE kmod_name){
	printf("Unloading %s kernel module...\n", RSTRING_PTR(kmod_name));

	int ret = 0, err;
	struct kmod_ctx *ctx;
	struct kmod_module *mod;
	struct kmod_list *list, *itr;
	const char *null_config = NULL;

	ctx = kmod_new(NULL, &null_config);
	if(ctx == NULL){
		ret = -1;
		printf("Unexpected error...\n");
	}else{
		err = kmod_module_new_from_name(ctx, RSTRING_PTR(kmod_name), &mod);
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
	return INT2NUM(ret);
}

//0 == ok, 1 == ko
static VALUE start(VALUE self){
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

	return INT2NUM(ret);
}

//0 == ok, 1 == ko
static VALUE stop(VALUE self){
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

	return INT2NUM(ret);
}