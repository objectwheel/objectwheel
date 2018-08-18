#ifndef GLOBALRESOURCESPANE_H
#define GLOBALRESOURCESPANE_H

#include <QWidget>
class GlobalResourcesPane : public QWidget
{
public:
    GlobalResourcesPane();

public slots:
    void sweep();

private:
    QSize sizeHint() const override;
};

#endif // GLOBALRESOURCESPANE_H