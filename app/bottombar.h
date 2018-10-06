#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include <QWidget>

class BottomBar : public QWidget
{
    Q_OBJECT
public:
    explicit BottomBar(QWidget *parent = nullptr);

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
};

#endif // BOTTOMBAR_H