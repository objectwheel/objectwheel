#ifndef VIEW_H
#define VIEW_H

#include <QHash>
#include <QWidget>

class QPropertyAnimation;
class QParallelAnimationGroup;

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

    protected:
        virtual void resizeEvent(QResizeEvent *event) override;

    private:
        void swipe(QWidget* w1, QWidget* w2, View::SwipeDirection direction);

    private:
        int m_visibleId;
        QHash<int, QWidget*> m_widgets;
        QParallelAnimationGroup* m_animationGroup;
        QPropertyAnimation* m_animationBack,* m_animationForth;
};

#endif // VIEW_H
