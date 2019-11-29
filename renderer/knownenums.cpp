#include <knownenums.h>
#include <QMetaObject>
#include <QMetaEnum>
#include <QRegularExpression>
#include <QDebug>
namespace KnownEnums {
namespace Internal {

class VideoOutput final {
    Q_GADGET
public:
    enum FlushMode
    {
        EmptyFrame,
        FirstFrame,
        LastFrame
    };
    Q_ENUM(FlushMode)
    enum FillMode
    {
        Stretch            = Qt::IgnoreAspectRatio,
        PreserveAspectFit  = Qt::KeepAspectRatio,
        PreserveAspectCrop = Qt::KeepAspectRatioByExpanding
    };
    Q_ENUM(FillMode)
};

QString cleanScopeName(QString scope)
{
    scope.replace("QQuickWindowQmlImpl", "Window");
    scope.replace("QQuick", "");
    scope.replace("QDeclarative", "");
    scope.replace("QQml", "");
    scope.replace("QWindow", "Window");
    scope.replace("KnownEnums::Internal::", "");
    return scope;
}
} // Internal

Enum extractEnum(const QMetaProperty& property, const QMetaEnum& me, const QObject* object)
{
    Enum e;
    e.name = property.name();
    e.scope = Internal::cleanScopeName(me.scope());
    e.value = me.valueToKey(property.read(object).toInt());
    for (int i = me.keyCount(); i--;) {
        if (QString(me.key(i)).contains(QRegularExpression("[^\\r\\n\\t\\f\\v ]")))
            e.keys[me.key(i)] = me.value(i);
    }
    return e;
}

bool isKnownEnum(const QString& identifier)
{
    static const QStringList knownIdentifiers {
        "VideoOutput.fillMode"
    };
    return knownIdentifiers.contains(identifier);
}

Enum knownEnum(const QMetaProperty& property, const QObject* object, const QString& identifier)
{
    if (identifier == "VideoOutput.fillMode")
        return extractEnum(property, QMetaEnum::fromType<Internal::VideoOutput::FillMode>(), object);
    return Enum();
}
} // KnownEnums

#include "knownenums.moc"