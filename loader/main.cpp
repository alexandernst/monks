#include "loader.h"
#include "loaderdbus.h"
#include <QCoreApplication>

#define PROCMON_PATH "./procmon.ko"
#define PROCMON_NAME "procmon"

int main(int argc, char *argv[]){

    QCoreApplication app(argc, argv);

    app.setApplicationName("lkm_loader");
    app.setOrganizationDomain("procmon");

    if(setuid(0) != 0){
        qCritical(QString("Could not set uid to 0!\nMake sure to run:\nsudo chown root '%1'\nsudo chmod ugo+xs '%1'").arg(QCoreApplication::applicationFilePath()).toLocal8Bit().data());
        return 1;
    }

    Loader *loader = new Loader();

    new LoaderDBus(loader);
    QDBusConnection connection = QDBusConnection::systemBus();
    if(!connection.registerService("com.procmon.loader")){
        qCritical("Could not register service!\nMake sure you copied com.procmon.loader.conf file to /etc/dbus-1/system.d/");
        return 1;
    }
    if(!connection.registerObject("/", loader)){
        qCritical("Count not register object!\nErr... I do not know why this happened. Try Google!");
        return 1;
    }

    return app.exec();

}
