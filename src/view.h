#ifndef VIEW_H
#define VIEW_H

#include <QMap>
#include <QWidget>
#include <QVBoxLayout>

class View : public QWidget
{
        Q_OBJECT
        typedef QMap<int, QWidget*> WidgetMap;

    public:
        explicit View(QWidget* parent = nullptr);
        QWidget* widget(int id) const;
        int id(QWidget* widget) const;
        int current() const;

    public slots:
        void show(int id);
        void add(int id, QWidget* widget);
        int remove(int id);

    signals:
        void visibleChanged();

    private:
        QVBoxLayout _layout;
        WidgetMap _widgets;
};

#endif // VIEW_H
