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
    static ControlPropertyManager* instance();
    static void setX(Control* control, qreal x, bool save = true,
                     bool updatePreviewer = true, bool compress = false);
    static void setY(Control* control, qreal y, bool save = true,
                     bool updatePreviewer = true, bool compress = false);
    static void setPos(Control* control, const QPointF& pos, bool save = true,
                       bool updatePreviewer = true, bool compress = false);
    static void setSize(Control* control, const QSizeF& size, bool save = true,
                        bool updatePreviewer = true, bool compress = false);
    static void setGeometry(Control* control, const QRectF& geometry, bool save = true,
                            bool updatePreviewer = true, bool compress = false);
    static void setParent(Control* control, Control* parentControl, bool save = true,
                          bool updatePreviewer = true, bool compress = false);
    static void setZ(Control* control, qreal z, bool save = true,
                     bool updatePreviewer = true, bool compress = false);
    static void setId(Control* control, const QString& id, bool save = true, bool updatePreviewer = true);

private slots:
    void handleDirtyProperties();

signals:
    void zChanged(Control*);
    void parentChanged(Control*);
    void geometryChanged(Control*);
    void idChanged(Control*, const QString& previousId);

private:
    explicit ControlPropertyManager(QObject* parent = nullptr);
    ~ControlPropertyManager();

private:
    static ControlPropertyManager* s_instance;
    static QTimer* s_dirtyPropertyProcessingTimer;
    static QList<DirtyProperty> s_dirtyProperties;
};

#endif // CONTROLPROPERTYMANAGER_H