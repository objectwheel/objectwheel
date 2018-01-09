#ifndef VIEW_H
#define VIEW_H

#include <QMap>
#include <QWidget>

class QLayout;

class View : public QWidget
{
        Q_OBJECT

    public:
        enum SwipeDirection {
            NoSwipe,
            LeftToRight,
            RightToLeft
        };

    public:
        explicit View(QWidget* parent = nullptr);
        void add(int id, QWidget* widget);

    public slots:
        void show(int id, SwipeDirection = NoSwipe);

    private:
        int _visibleId;
        QLayout* _layout;
        QMap<int, QWidget*> _widgets;
};

#endif // VIEW_H
