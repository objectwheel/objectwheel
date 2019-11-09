#include <objectwheel.h>
#include <translation.h>

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class ObjectwheelModule final : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_DISABLE_COPY(ObjectwheelModule)
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    ObjectwheelModule(QObject* parent = nullptr) : QQmlExtensionPlugin(parent) {}

    void registerTypes(const char* uri) override
    {
        Q_ASSERT(QLatin1String(uri) == QLatin1String("Objectwheel"));

        const int major = OW_MODULE_VERSION_MAJOR;
        const int minor = OW_MODULE_VERSION_MINOR;

        // 1.0 types
        qmlRegisterSingletonType<Objectwheel>(uri, major, minor, Objectwheel::staticMetaObject.className(),
                                              [] (QQmlEngine* e, QJSEngine*) -> QObject* {
            return new Objectwheel(e);
        });
        qmlRegisterSingletonType<Translation>(uri, major, minor, Translation::staticMetaObject.className(),
                                              [] (QQmlEngine* e, QJSEngine*) -> QObject* {
            return new Translation(e);
        });

        qmlRegisterModule(uri, major, minor);
    }
};

#include "objectwheelmodule.moc"
