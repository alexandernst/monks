#include "procmonui.h"
#include "procmonuidbus.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    ProcmonUI *procmon = new ProcmonUI;

    new ProcmonDBus(procmon);
    QDBusConnection connection = QDBusConnection::systemBus();
    connection.registerService("com.procmon.procmonui");
    connection.registerObject("/", procmon);

    if(!connection.registerService("com.procmon.procmonui")){
        QMessageBox::critical(NULL, "Error", "Could not register service!\nMake sure you copied com.procmon.procmonui.conf file to /etc/dbus-1/system.d/");
        return 1;
    }
    if(!connection.registerObject("/", procmon)){
        QMessageBox::critical(NULL, "Error", "Count not register object!\nErr... I do not know why this happened. Try Google!");
        return 1;
    }

    procmon->show();
    return app.exec();
}
