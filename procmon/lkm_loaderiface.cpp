#include "lkm_loaderiface.h"

LKM_LoaderIface::LKM_LoaderIface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent) : QDBusAbstractInterface(service, path, staticInterfaceName(), connection, parent){
}

LKM_LoaderIface::~LKM_LoaderIface(){
}

