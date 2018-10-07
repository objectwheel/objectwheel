#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include <QWidget>

class QHBoxLayout;
class PushButton;

class BottomBar : public QWidget
{
    Q_OBJECT
public:
    explicit BottomBar(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QHBoxLayout* m_layout;
    PushButton* m_consoleButton;
};

#endif // BOTTOMBAR_H