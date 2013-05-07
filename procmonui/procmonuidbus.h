#ifndef PROCMONUIDBUS_H
#define PROCMONUIDBUS_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

class ProcmonDBus: public QDBusAbstractAdaptor{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.procmon.procmonui")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name='com.procmon.procmonui'>               \n"
                "    <method name=\"add_syscall_data\">                   \n"
                "      <arg direction=\"in\" type=\"s\" name=\"data\"/>   \n"
                "    </method>                                            \n"
                "  </interface>                                           \n")

	public:
	    ProcmonDBus(QObject *parent);
	    virtual ~ProcmonDBus();

	public Q_SLOTS:
	    void add_syscall_data(const QString &test);
};

#endif
