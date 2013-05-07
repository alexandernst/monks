#ifndef LKM_LOADERDBUS_H
#define LKM_LOADERDBUS_H

#include <QObject>
#include <QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

class LoaderDBus: public QDBusAbstractAdaptor{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.procmon.loader")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name='com.procmon.loader'>                  \n"
                "    <method name='check'>                                \n"
                "      <arg direction='out' type='i' name='ret' />        \n"
                "    </method>                                            \n"
                "    <method name='load'>                                 \n"
                "      <arg direction='out' type='i' name='ret' />        \n"
                "    </method>                                            \n"
                "    <method name='unload'>                               \n"
                "      <arg direction='out' type='i' name='ret' />        \n"
                "    </method>                                            \n"
                "  </interface>                                           \n")
    public:
        LoaderDBus(QObject *parent);
        virtual ~LoaderDBus();

    public Q_SLOTS:
        int check();
        int load();
        int unload();
};

#endif
