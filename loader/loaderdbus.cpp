#include "lkm_loaderdbus.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

LoaderDBus::LoaderDBus(QObject *parent) : QDBusAbstractAdaptor(parent){
    setAutoRelaySignals(true);
}

LoaderDBus::~LoaderDBus(){
}

int LoaderDBus::check(){
    int ret;
    QMetaObject::invokeMethod(parent(), "check", Q_RETURN_ARG(int, ret));
    return ret;
}

int LoaderDBus::load(){
    int ret;
    QMetaObject::invokeMethod(parent(), "load", Q_RETURN_ARG(int, ret));
    return ret;
}

int LoaderDBus::unload(){
    int ret;
    QMetaObject::invokeMethod(parent(), "unload", Q_RETURN_ARG(int, ret));
    return ret;
}
