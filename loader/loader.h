#ifndef LKM_LOADER_H
#define LKM_LOADER_H

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <libkmod.h>

#include <QCoreApplication>

#define PROCMON_PATH "./procmon.ko"
#define PROCMON_NAME "procmon"

class Loader : public QObject{
    Q_OBJECT

    public Q_SLOTS:
        int load();
        int unload();
        int check();

    private:
        int ret = 0, err;
        struct kmod_ctx *ctx;
        struct kmod_module *mod;
        struct kmod_list *list, *itr;
        const char *null_config = NULL;

};

#endif // LKM_LOADER_H
