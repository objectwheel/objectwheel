#ifndef OUTPUTBOX_H
#define OUTPUTBOX_H

#include <global.h>
#include <QWidget>
#include <QMap>

class QSplitterHandle;
class OutputBoxPrivate;

class OutputBox : public QWidget
{
        Q_OBJECT
    public:
        explicit OutputBox(QWidget *parent = nullptr);
        void showPane(PaneType pane);
        void setSplitterHandle(QSplitterHandle* splitterHandle);

    public slots:
        void hidePanes();
        void showPanes();

    private:
        QSplitterHandle* _splitterHandle;
        OutputBoxPrivate* _d;
        int _lastHeight;
};

#endif // OUTPUTBOX_H
