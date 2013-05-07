#include "ui.h"
#include "uidbus.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    Procmon *procmon = new Procmon;

    new ProcmonDBus(procmon);
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("com.procmon.procmon");
    connection.registerObject("/", procmon);

    procmon->show();
    return app.exec();
}
