#include "procmon.h"
#include "ui_procmon.h"

//#include <unistd.h>
#include <QFile>
#include <QDebug>
#include <QObject>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>

#define PROCMON_PATH "/proc/procmon"

Procmon::Procmon(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);

    lkm_loaderiface = new LKM_LoaderIface("com.procmon.lkm_loader", "/", QDBusConnection::systemBus(), 0);

    if(!lkm_loaderiface->isValid()){
        QProcess *lkm_loader = new QProcess();
        qDebug() << QString("%1/lkm_helper").arg(QFileInfo(QDir::currentPath()).absolutePath());

        lkm_loader->startDetached(QString("%1/lkm_helper").arg(QFileInfo(QDir::currentPath()).absolutePath()));
    }

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
        if(change_loaded_state(1) != -1){
            change_btns_state(LOADED);
        }else{
            QMessageBox::critical(NULL, "Error", "Error loading kernel module!");
        }
    });

    connect(ui->btn_start, &QPushButton::clicked, [this](){
        if(change_running_state(1) != -1){
            change_btns_state(STARTED);
        }else{
            QMessageBox::critical(NULL, "Error", "Error starting kernel module!");
        }
    });

    connect(ui->btn_stop, &QPushButton::clicked, [this](){
        if(change_running_state(0) != -1){
            change_btns_state(STOPPED);
        }else{
            QMessageBox::critical(NULL, "Error", "Error stoping kernel module!");
        }
    });

    connect(ui->btn_unload, &QPushButton::clicked, [this](){
        if(change_loaded_state(0) != -1){
            change_btns_state(UNLOADED);
        }else{
            QMessageBox::critical(NULL, "Error", "Error unloading kernel module!");
        }
    });

}

Procmon::~Procmon(){
    delete ui;
}

//state values: 1 = load, 0 = unload, -1 = check if loaded
//return values: -1 = error, 0 = not loaded, 1 = loaded
int Procmon::change_loaded_state(int state){
    QDBusMessage msg;
    if(state == -1){
        msg = lkm_loaderiface->call("check");
    }else if(state == 0){
        msg = lkm_loaderiface->call("unload");
    }else if(state == 1){
        msg = lkm_loaderiface->call("load");
    }

    qDebug() << msg.arguments().at(0).toInt();
    return msg.arguments().at(0).toInt();
}

//return values: -1 = error, 0 = ok
int Procmon::change_running_state(int state){

    int ret = 0;
    QFile file(PROCMON_PATH);
    if (!file.open(QIODevice::WriteOnly)){
        QMessageBox::critical(NULL, "Error", QString("Error writing to %1").arg(PROCMON_PATH));
        ret = -1;
    }else{
        //char str[2];
        //sprintf(str, "%d", state);
        //file.write(&str[0]);
        QByteArray *ba = new QByteArray();
        ba->append(QString("%1").arg(state));
        file.write(ba->data());
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

void Procmon::add_syscall_data(const QString &data){

    qDebug() << data;

}
