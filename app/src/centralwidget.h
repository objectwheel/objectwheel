#ifndef CENTRALWIDGET_H
#define CENTRALWIDGET_H

#include <QMainWindow>
#include <QMap>
#include <QVBoxLayout>

typedef QMap<int, QWidget*> WidgetMap;

class CentralWidget : public QWidget
{
        Q_OBJECT

    public:
        explicit CentralWidget(QMainWindow* parent);
        void addWidget(int uid, QWidget* widget);
        int removeWidget(int uid);
        int uid(QWidget* widget) const;
        QWidget* widget(int uid) const;
        int visibleUid() const;

    public slots:
        void showWidget(int uid);

    signals:
        void visibleWidgetChanged();

    private:
        QVBoxLayout _layout;
        WidgetMap _widgetMap;
};

#endif // CENTRALWIDGET_H
