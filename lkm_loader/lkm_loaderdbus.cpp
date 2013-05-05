#include "lkm_loaderdbus.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

LKM_LoaderDBus::LKM_LoaderDBus(QObject *parent) : QDBusAbstractAdaptor(parent){
    setAutoRelaySignals(true);
}

LKM_LoaderDBus::~LKM_LoaderDBus(){
}

int LKM_LoaderDBus::check(){
    int ret;
    QMetaObject::invokeMethod(parent(), "check", Q_RETURN_ARG(int, ret));
    return ret;
}

int LKM_LoaderDBus::load(){
    int ret;
    QMetaObject::invokeMethod(parent(), "load", Q_RETURN_ARG(int, ret));
    return ret;
}

int LKM_LoaderDBus::unload(){
    int ret;
    QMetaObject::invokeMethod(parent(), "unload", Q_RETURN_ARG(int, ret));
    return ret;
}
