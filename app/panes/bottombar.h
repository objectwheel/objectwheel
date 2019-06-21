#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include <pushbutton.h>

class QHBoxLayout;
class QButtonGroup;
class ButtonFlasher;

class BottomBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BottomBar)

public:
    explicit BottomBar(QWidget* parent = nullptr);

    QAbstractButton* activeButton() const;
    QAbstractButton* consoleButton() const;
    QAbstractButton* issuesButton() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void discharge();
    void flash(QAbstractButton*);

private slots:
    void onButtonClick(QAbstractButton* button);

protected:
    void paintEvent(QPaintEvent*) override;

signals:
    void buttonActivated(QAbstractButton* button, bool checked);

private:
    QHBoxLayout* m_layout;
    QButtonGroup* m_buttonGroup;
    PushButton* m_consoleButton;
    PushButton* m_issuesButton;
    ButtonFlasher* m_consoleFlasher;
    ButtonFlasher* m_issuesFlasher;
};

#endif // BOTTOMBAR_H