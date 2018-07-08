#ifndef CONTROLPROPERTYMANAGER_H
#define CONTROLPROPERTYMANAGER_H

#include <QObject>
#include <QHash>

class Control;
class QTimer;

class ControlPropertyManager final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(ControlPropertyManager)

    friend class ApplicationCore;

public:
    static std::function<bool()> defaultZPropertyConditions(const Control* control);
    static std::function<bool()> defaultParentPropertyConditions(const Control* control, const Control* parentControl);

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

    static void setId(Control* control, const QString& id, bool save = true,
                      bool updatePreviewer = true, bool compress = false);
    static void setParent(Control* control, Control* parentControl, bool save = true,
                          bool updatePreviewer = true, bool compress = false);
    static void setZ(Control* control, qreal z, bool save = true,
                     bool updatePreviewer = true, bool compress = false);

signals:
    void zChanged(Control*);
    void parentChanged(Control*);
    void geometryChanged(Control*);
    void idChanged(Control*, const QString& previousId);

    void previewChanged(Control*);
    void errorOccurred(Control*);
    void doubleClicked(Control*);
    void controlDropped(Control*, const QPointF&, const QString&);

private:
    static bool zPropertyConditions(const Control* control);
    static bool parentPropertyConditions(const Control* control, const Control* parentControl);

private:
    explicit ControlPropertyManager(QObject* parent = nullptr);
    ~ControlPropertyManager();

private:
    static ControlPropertyManager* s_instance;
    static QTimer* s_dirtyPropertyProcessingTimer;
    static QHash<QString, std::function<void()>> s_dirtyPropertyHandlingFunctions;
};

#endif // CONTROLPROPERTYMANAGER_H