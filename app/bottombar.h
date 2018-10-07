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
    PushButton* consoleButton() const;
    PushButton* issuesButton() const;

public:
    void flash(PushButton*);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QHBoxLayout* m_layout;
    PushButton* m_consoleButton;
    PushButton* m_issuesButton;
};

#endif // BOTTOMBAR_H