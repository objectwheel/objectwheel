#ifndef CONTROLPROPERTYMANAGER_H
#define CONTROLPROPERTYMANAGER_H

#include <QObject>
#include <QList>

class Control;
class QTimer;

class ControlPropertyManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlPropertyManager)

    friend class ApplicationCore; // For constructor and destructor

    struct DirtyProperty {
        QString key;
        std::function<void()> function;
        bool operator==(const DirtyProperty& b) const
        {return key == b.key; }
    };

public:
    enum Option {
        NoOption = 0x0,
        SaveChanges = 0x1,
        UpdateRenderer = 0x2,
        CompressedCall = 0x4,
        DontApplyDesigner = 0x8
    };
    Q_DECLARE_FLAGS(Options, Option)

public:
    static ControlPropertyManager* instance();
    static void setX(Control* control, qreal x, Options options, const QString& geometrySyncKey = QString()); // Set x, based on scene coord jargon
    static void setY(Control* control, qreal y, Options options, const QString& geometrySyncKey = QString()); // Set y, based on scene coord jargon
    static void setZ(Control* control, qreal z, Options options);
    static void setWidth(Control* control, qreal width, Options options, const QString& geometrySyncKey = QString());
    static void setHeight(Control* control, qreal height, Options options, const QString& geometrySyncKey = QString());
    static void setPos(Control* control, const QPointF& pos, Options options, const QString& geometrySyncKey = QString()); // Set pos, based on scene coord jargon
    static void setSize(Control* control, const QSizeF& size, Options options, const QString& geometrySyncKey = QString());
    static void setGeometry(Control* control, const QRectF& geometry, Options options, const QString& geometrySyncKey = QString()); // Set geometry, based on scene coord jargon
    static void setParent(Control* control, Control* parentControl, Options options);
    static void setId(Control* control, const QString& id, Options options);
    static void setIndex(Control* control, quint32 index, Options options);
    static void setProperty(Control* control, const QString& propertyName, const QString& parserValue,
                            const QVariant& propertyValue, Options options);

private slots:
    void handleDirtyProperties();

private:
    static QVariant geometrySyncValue(const QVariant& value, const QString& hash = QString());

signals:
    void zChanged(Control*);
    void parentChanged(Control*);
    void geometryChanged(Control*);
    void doubleClicked(Control*, Qt::MouseButtons);
    void renderInfoChanged(Control*, bool codeChanged);
    void indexChanged(Control*);
    void idChanged(Control*, const QString& previousId);
    void propertyChanged(Control*, const QString& propertyName);

private:
    explicit ControlPropertyManager(QObject* parent = nullptr);
    ~ControlPropertyManager() override;

private:
    static ControlPropertyManager* s_instance;
    static QTimer* s_dirtyPropertyProcessingTimer;
    static QList<DirtyProperty> s_dirtyProperties;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ControlPropertyManager::Options)

#endif // CONTROLPROPERTYMANAGER_H