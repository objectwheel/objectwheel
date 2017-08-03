#ifndef CONTROLSSCROLLPANEL_H
#define CONTROLSSCROLLPANEL_H

#include <control.h>
#include <QGraphicsWidget>
#include <QMarginsF>

class ControlsScrollPanel;
class ControlsScrollPanelPrivate;

class ControlsScrollBar : public QGraphicsWidget
{
        Q_OBJECT
    public:
        explicit ControlsScrollBar(QGraphicsWidget* parent = 0);

        Qt::Orientation orientation() const;
        void setOrientation(const Qt::Orientation& orientation);

    private:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    private:
        Qt::Orientation _orientation;
};

class ControlsScrollPanel : public QGraphicsWidget
{
        Q_OBJECT
        friend class ControlsScrollPanelPrivate;

    public:
        explicit ControlsScrollPanel(QGraphicsWidget *parent = 0);

        Qt::Orientation orientation() const;
        void setOrientation(const Qt::Orientation& orientation);

        QList<Control*> controls() const;
        void addControl(Control* control);
        void removeControl(Control* control);

        QMarginsF margins() const;
        void setMargins(const QMarginsF& margins);

        int spacing() const;
        void setSpacing(int spacing);

        Qt::ScrollBarPolicy horizontalScrollBarPolicy() const;
        void setHorizontalScrollBarPolicy(const Qt::ScrollBarPolicy& horizontalScrollBarPolicy);

        Qt::ScrollBarPolicy verticalScrollBarPolicy() const;
        void setVerticalScrollBarPolicy(const Qt::ScrollBarPolicy& verticalScrollBarPolicy);

        bool showIds() const;
        void setShowIds(bool showIds);

    private:
        void resizeEvent(QGraphicsSceneResizeEvent *event) override;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = Q_NULLPTR) override;

    private:
        ControlsScrollPanelPrivate* _d;
        QGraphicsWidget _viewport;
        Qt::Orientation _orientation;
        Qt::Alignment _alignment;
        QList<Control*> _controls;
        QMarginsF _margins;
        int _spacing;
        Qt::ScrollBarPolicy _horizontalScrollBarPolicy;
        Qt::ScrollBarPolicy _verticalScrollBarPolicy;
        bool _showIds;
        ControlsScrollBar _horizontalScrollBar;
        ControlsScrollBar _verticalScrollBar;
};

#endif // CONTROLSSCROLLPANEL_H
