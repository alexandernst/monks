#ifndef LOADERIFACE_H
#define LOADERIFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

class LoaderIface: public QDBusAbstractInterface{
    Q_OBJECT
    public:
        static inline const char *staticInterfaceName(){
            return "com.procmon.lkm_loader";
        }

    public:
        LoaderIface(const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent = 0);
        ~LoaderIface();

    public Q_SLOTS:
        inline QDBusPendingReply<int> check(){
            return asyncCall(QLatin1String("check"));
        }

        inline QDBusPendingReply<int> load(){
            return asyncCall(QLatin1String("load"));
        }

        inline QDBusPendingReply<int> unload(){
            return asyncCall(QLatin1String("unload"));
        }
};

namespace com {
  namespace procmon {
    typedef ::LoaderIface lkm_loader;
  }
}
#endif
