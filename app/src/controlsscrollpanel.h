#ifndef CONTROLSSCROLLPANEL_H
#define CONTROLSSCROLLPANEL_H

#include <control.h>
#include <QGraphicsWidget>
#include <QMarginsF>
#include <QScrollBar>

class ControlsScrollPanelPrivate;

class ControlsScrollPanel : public QGraphicsWidget
{
        Q_OBJECT
        friend class ControlsScrollPanelPrivate;

    public:
        explicit ControlsScrollPanel(QGraphicsScene* scene, QGraphicsWidget *parent = 0);

        Qt::Orientation orientation() const;
        void setOrientation(const Qt::Orientation& orientation);

        QList<Control*> controls() const;
        void addControl(Control* control);
        Q_INVOKABLE void removeControl(Control* control);

        QMarginsF margins() const;
        void setMargins(const QMarginsF& margins);

        qreal spacing() const;
        void setSpacing(qreal spacing);

        Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
        void setHorizontalScrollBarPolicy(const Qt::ScrollBarPolicy& horizontalScrollBarPolicy);

        Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
        void setVerticalScrollBarPolicy(const Qt::ScrollBarPolicy& verticalScrollBarPolicy);

        bool showIds() const;
        void setShowIds(bool showIds);

        QGraphicsWidget* viewport();

    public slots:
        void clear();

    private slots:
        void handleHBarValueChange(int value);
        void handleVBarValueChange(int value);

    private:
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;
        void wheelEvent(QGraphicsSceneWheelEvent *event) override;

    private:
        ControlsScrollPanelPrivate* _d;
        QGraphicsWidget _viewport;
        Qt::Orientation _orientation;
        QList<Control*> _controls;
        QMarginsF _margins;
        qreal _spacing;
        Qt::ScrollBarPolicy _horizontalScrollBarPolicy;
        Qt::ScrollBarPolicy _verticalScrollBarPolicy;
        bool _showIds;
        QScrollBar _horizontalScrollBar;
        QScrollBar _verticalScrollBar;
};

#endif // CONTROLSSCROLLPANEL_H
