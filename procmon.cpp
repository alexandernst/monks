#include "procmon.h"
#include "ui_procmon.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>

#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <libkmod.h>

#define PROCMON_PATH "/proc/procmon"
#define PROCMON_NAME "procmon"

Procmon::Procmon(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    if(change_loaded_state(-1) == 0){
        change_btns_state(UNLOADED);
    }else{
        QFile file(PROCMON_PATH);
        if (!file.open(QIODevice::ReadOnly)){
            QMessageBox::critical(NULL, "Error", QString("Error writing to %1").arg(PROCMON_PATH));
        }else{
            if(file.readAll() == "1"){
                change_btns_state(STARTED);
            }else{
                change_btns_state(STOPPED);
            }
        }
        file.close();
    }

    connect(ui->btn_load, &QPushButton::clicked, [this](){
        if(change_loaded_state(1) != -1)
            change_btns_state(LOADED);
    });

    connect(ui->btn_start, &QPushButton::clicked, [this](){
        if(change_running_state(1) != -1)
            change_btns_state(STARTED);
    });

    connect(ui->btn_stop, &QPushButton::clicked, [this](){
        if(change_running_state(0) != -1)
            change_btns_state(STOPPED);
    });

    connect(ui->btn_unload, &QPushButton::clicked, [this](){
        if(change_loaded_state(0) != -1)
            change_btns_state(UNLOADED);
    });

}

Procmon::~Procmon(){
    delete ui;
}

//state values: 1 = load, 0 = unload, -1 = check if loaded
//return values: -1 = error, 0 = not loaded, 1 = loaded
int Procmon::change_loaded_state(int state){
    int ret = 0, err;

    __uid_t uid = getuid();
    setuid(0);

    struct kmod_ctx *ctx;
    struct kmod_module *mod;
    const char *null_config = NULL;

    ctx = kmod_new(NULL, &null_config);
    if(ctx == NULL){
        QMessageBox::critical(NULL, "Error", "Error starting kmod");
        return -1;
    }

    if(state == 1){
        err = kmod_module_new_from_path(ctx, "./procmon.ko", &mod);
        if(err != 0){
            QMessageBox::critical(NULL, "Error", QString("Error creating module from path").arg(strerror(-err)));
            ret = -1;
        }else{
            err = kmod_module_insert_module(mod, 0, NULL);
            if(err != 0){
                QMessageBox::critical(NULL, "Error", QString("Error inserting module").arg(strerror(-err)));
                ret = -1;
            }
        }
    }else if(state == 0){
        err = kmod_module_new_from_name(ctx, PROCMON_NAME, &mod);
        if(err != 0){
            QMessageBox::critical(NULL, "Error", QString("Error creating module from name").arg(strerror(-err)));
            ret = -1;
        }else{
            err = kmod_module_remove_module(mod, 0);
            if(err != 0){
                QMessageBox::critical(NULL, "Error", QString("Error removing module (%1)").arg(strerror(-err)));
                ret = -1;
            }
        }
    }else if(state == -1){
        struct kmod_list *list, *itr;
        err = kmod_module_new_from_loaded(ctx, &list);
        if(err < 0){
            QMessageBox::critical(NULL, "Error", QString("Error starting kmod").arg(strerror(-err)));
            ret = -1;
        }else{
            kmod_list_foreach(itr, list){
                mod = kmod_module_get_module(itr);
                const char *name = kmod_module_get_name(mod);
                if(strcmp(name, PROCMON_NAME) == 0){
                    ret = 1;
                }
                kmod_module_unref(mod);
            }
            kmod_module_unref_list(list);
        }
    }

    kmod_unref(ctx);
    setuid(uid);

    return ret;
}

//return values: -1 = error, 0 = ok
int Procmon::change_running_state(int state){

    int ret = 0;
    QFile file(PROCMON_PATH);
    if (!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(NULL, "Error", QString("Error writing to %1").arg(PROCMON_PATH));
        ret = -1;
    }else{
        char str[2];
        sprintf(str, "%d", state);
        file.write(&str[0]);
        ret = 0;
    }
    file.close();
    return ret;

}

void Procmon::change_btns_state(int states[]){
    ui->btn_load->setEnabled(states[0]);
    ui->btn_start->setEnabled(states[1]);
    ui->btn_stop->setEnabled(states[2]);
    ui->btn_unload->setEnabled(states[3]);
}
