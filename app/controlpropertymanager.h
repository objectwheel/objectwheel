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

public:
    static ControlPropertyManager* instance();
    static void setX(Control* control, qreal x, bool save = true,
                     bool updatePreviewer = true, bool compress = false,
                     std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setY(Control* control, qreal y, bool save = true,
                     bool updatePreviewer = true, bool compress = false,
                     std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setPos(Control* control, const QPointF& pos, bool save = true,
                       bool updatePreviewer = true, bool compress = false,
                       std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setSize(Control* control, const QSizeF& size, bool save = true,
                        bool updatePreviewer = true, bool compress = false,
                        std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setGeometry(Control* control, const QRectF& geometry, bool save = true,
                            bool updatePreviewer = true, bool compress = false,
                            std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setId(Control* control, const QString& id, bool save = true,
                      bool updatePreviewer = true, bool compress = false,
                      std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setParent(Control* control, Control* parentControl, bool save = true,
                          bool updatePreviewer = true, bool compress = false,
                          std::function<bool()> extraConditions = std::function<bool()>{nullptr});
    static void setZ(Control* control, qreal z, bool save = true,
                     bool updatePreviewer = true, bool compress = false,
                     std::function<bool()> extraConditions = std::function<bool()>{nullptr});

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

private:
    explicit ControlPropertyManager(QObject* parent = nullptr);
    ~ControlPropertyManager();

private:
    static ControlPropertyManager* s_instance;
    static QTimer* s_dirtyPropertyProcessingTimer;
    static QHash<QString, std::function<void()>> s_dirtyPropertyHandlingFunctions;
};

#endif // CONTROLPROPERTYMANAGER_H