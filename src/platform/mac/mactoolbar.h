#ifndef MACTOOLBAR_H
#define MACTOOLBAR_H

#include <QObject>

class QTimer;
class QMainWindow;

class MacToolbar : public QObject
{
        Q_OBJECT

    public:
        explicit MacToolbar(QMainWindow* mainWindow);

    private slots:
        void hideTitlebar();

    private:
        QMainWindow* _mainWindow;
        QTimer* _timer;
        qreal _toolbarHeight;
};

#endif // MACTOOLBAR_H