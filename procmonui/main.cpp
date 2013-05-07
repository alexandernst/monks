#include "procmonui.h"
#include "procmonuidbus.h"
#include <QApplication>

int main(int argc, char *argv[]){
    QApplication app(argc, argv);

    ProcmonUI *procmon = new ProcmonUI;

    new ProcmonDBus(procmon);
    QDBusConnection connection = QDBusConnection::systemBus();
    connection.registerService("com.procmon.procmonui");
    connection.registerObject("/", procmon);

    procmon->show();
    return app.exec();
}
