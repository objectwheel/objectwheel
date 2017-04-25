#ifndef BUBBLEHEAD_H
#define BUBBLEHEAD_H

#include <QIcon>
#include <QPushButton>
#include <QSharedPointer>

struct Callback {
        virtual ~Callback() = 0;
        virtual void call() = 0;

        QIcon icon;
        QPainterPath path;
};
inline Callback::~Callback() {}

template <typename T>
struct Button : public Callback
{
    virtual void call() { (object->*method)(); }

    T* object;
    void (T::* method)();
};

class BubbleHeadPrivate;

class BubbleHead : public QPushButton
{
		Q_OBJECT
        friend class BubbleHeadPrivate;

	public:
		explicit BubbleHead(QWidget *parent = 0);
		~BubbleHead();

        static void setIcon(const QIcon& icon);
        static const QIcon& icon();

        template<typename T>
        void addButton(const QIcon& icon, void* object, void (T::*method)()) {
            Button<T>* button = new Button<T>;
            button->icon = icon;
            button->object = (T*)object;
            button->method = method;
            m_buttonList.append(QSharedPointer<Callback>(button));
            updatePathes();
        }

    protected:
		void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
		void mouseReleaseEvent(QMouseEvent *event) override;
        void paintEvent(QPaintEvent *event) override;
		void moveEvent(QMoveEvent *event) override;

	signals:
		void moved(const QPoint&) const;

	private:
        void updatePathes();
        void updateGeometry();
        static BubbleHeadPrivate* m_d;
        QList<QSharedPointer<Callback>> m_buttonList;
};

#endif // BUBBLEHEAD_H
