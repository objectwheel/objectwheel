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
        UpdatePreviewer = 0x2,
        CompressedCall = 0x4,
        DontApplyDesigner = 0x8
    };
    Q_DECLARE_FLAGS(Options, Option)

public:
    static ControlPropertyManager* instance();
    static void setX(Control* control, qreal x, Options options); // Set x, based on scene coord jargon
    static void setY(Control* control, qreal y, Options options); // Set y, based on scene coord jargon
    static void setZ(Control* control, qreal z, Options options);
    static void setWidth(Control* control, qreal width, Options options);
    static void setHeight(Control* control, qreal height, Options options);
    static void setPos(Control* control, const QPointF& pos, Options options); // Set pos, based on scene coord jargon
    static void setSize(Control* control, const QSizeF& size, Options options);
    static void setGeometry(Control* control, const QRectF& geometry, Options options); // Set geometry, based on scene coord jargon
    static void setParent(Control* control, Control* parentControl, Options options);
    static void setId(Control* control, const QString& id, Options options);
    static void setProperty(Control* control, const QString& propertyName, const QString& parserValue,
                            const QVariant& propertyValue, Options options);

    static qreal xWithMargin(const Control* control, qreal x, bool add);
    static qreal yWithMargin(const Control* control, qreal y, bool add);
    static QPointF posWithMargin(const Control* control, const QPointF& pos, bool add);
    static QRectF geoWithMargin(const Control* control, const QRectF& geo, bool add);

private slots:
    void handleDirtyProperties();

signals:
    void zChanged(Control*);
    void parentChanged(Control*);
    void geometryChanged(Control*);
    void previewChanged(Control*, bool codeChanged);
    void idChanged(Control*, const QString& previousId);
    void propertyChanged(Control*, const QString& propertyName);

private:
    explicit ControlPropertyManager(QObject* parent = nullptr);
    ~ControlPropertyManager();

private:
    static ControlPropertyManager* s_instance;
    static QTimer* s_dirtyPropertyProcessingTimer;
    static QList<DirtyProperty> s_dirtyProperties;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(ControlPropertyManager::Options)

#endif // CONTROLPROPERTYMANAGER_H