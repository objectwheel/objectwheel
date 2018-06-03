#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QAbstractButton>

class ToolButton : public QAbstractButton
{
    Q_OBJECT
public:
    explicit ToolButton(QWidget* parent = nullptr);

private:
    QSize sizeHint() const override;
    void enterEvent(QEvent* e) override;
    void leaveEvent(QEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    bool m_hoverOver;
};

#endif // TOOLBUTTON_H