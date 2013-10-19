#ifndef LKM_H
#define LKM_H

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <libkmod.h>

int check(char *kmod_name);
int load(char *kmod_path);
int unload(char *kmod_name);
int start();
int stop();

#endif