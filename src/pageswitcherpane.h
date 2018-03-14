#ifndef PAGESWITCHERPANE_H
#define PAGESWITCHERPANE_H

#include <QWidget>

class PageSwitcherPane : public QWidget
{
        Q_OBJECT

    public:
        explicit PageSwitcherPane(QWidget *parent = nullptr);

    protected:
        void paintEvent(QPaintEvent *event) override;
};

#endif // PAGESWITCHERPANE_H