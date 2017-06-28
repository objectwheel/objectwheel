#ifndef CONTROL_H
#define CONTROL_H

#include <QGraphicsWidget>
#include <QUrl>
#include <QList>
#include <QPixmap>

class ControlPrivate;
class PagePrivate;

class Control : public QGraphicsWidget
{
        Q_OBJECT

    public:
        explicit Control(Control* parent = Q_NULLPTR);
        virtual ~Control();

        QString id() const;
        void setId(const QString& id);

        QUrl url() const;
        void setUrl(const QUrl& url);

        static bool showOutline();
        static void setShowOutline(const bool value);

        QList<Control*> childControls() const;
        Control* parentControl() const;

    protected:
        virtual void refresh();
        virtual void dragEnterEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dragMoveEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    protected:
        ControlPrivate* _d;

    private:
        QString _id;
        QUrl _url;
        static bool _showOutline;
};

class Page : public Control
{
        Q_OBJECT

    public:
        struct SkinSetting {
                SkinSetting() : resizable(true) {}
                SkinSetting(const QPixmap& p, const QRectF& r, bool b)
                    : pixmap(p), rect(r), resizable(b) {}
                QPixmap pixmap;
                QRectF rect;
                bool resizable;
        };

        explicit Page(Page* parent = Q_NULLPTR);

        bool mainPage() const;
        void setMainPage(bool mainPage);

        static void setSkinSetting(const SkinSetting* skinSetting);
        static const SkinSetting* skinSetting();

        bool resizable() const;
        void setResizable(bool resizable);

        bool stickSelectedControlToGuideLines() const;
        QVector<QLineF> guideLines() const;

    public slots:
        void centralize();

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

    private:
        PagePrivate* _d;
        bool _mainPage = false;
        QList<Control*> _controls;
        static const SkinSetting* _skinSetting;
        bool _resizable;
};

#endif // CONTROL_H
