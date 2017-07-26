#ifndef DESIGNMANAGER_H
#define DESIGNMANAGER_H

#include <QObject>

class QWidget;
class DesignManagerPrivate;

class DesignManager : public QObject
{
        Q_OBJECT
        Q_DISABLE_COPY(DesignManager)

    private:
        static DesignManagerPrivate* _d;

    public:
        explicit DesignManager(QObject *parent = 0);
        static DesignManager* instance();
        static void setSettleWidget(QWidget* widget);
        static void showWidget();
        static void hideWidget();

};

#endif // DESIGNMANAGER_H
