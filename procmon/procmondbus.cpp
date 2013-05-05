#include "procmondbus.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

ProcmonDBus::ProcmonDBus(QObject *parent) : QDBusAbstractAdaptor(parent){
	setAutoRelaySignals(true);
}

ProcmonDBus::~ProcmonDBus(){
}

void ProcmonDBus::add_syscall_data(const QString &data){
	QMetaObject::invokeMethod(parent(), "add_syscall_data", Q_ARG(QString, data));
}
