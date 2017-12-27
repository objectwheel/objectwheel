#ifndef MACTOOLBAR_H
#define MACTOOLBAR_H

#include <QObject>

#if defined(__OBJC__)
@class NSWindow;
#else
class NSWindow;
#endif

class QMainWindow;

class MacToolbar : public QObject
{
        Q_OBJECT

    public:
        explicit MacToolbar(QMainWindow* mainWindow);
        qreal toolbarHeight() const;

    private:
        qreal _toolbarHeight;
};

#endif // MACTOOLBAR_H