#ifndef PAGESWITCHERPANE_H
#define PAGESWITCHERPANE_H

#include <QWidget>

class PageSwitcherPane : public QWidget
{
        Q_OBJECT

    public:
        explicit PageSwitcherPane(QWidget *parent = nullptr);
};

#endif // PAGESWITCHERPANE_H