#ifndef OUTPUTBAR_H
#define OUTPUTBAR_H

#include <pushbutton.h>

class ButtonFlasher;
class OutputBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(OutputBar)

public:
    explicit OutputBar(QWidget* parent = nullptr);

    QAbstractButton* activeButton() const;
    QAbstractButton* consoleButton() const;
    QAbstractButton* issuesButton() const;

public slots:
    void flash(QAbstractButton* button);

private slots:
    void onButtonClick(bool checked);

private:
    void paintEvent(QPaintEvent* event) override;

signals:
    void buttonActivated(QAbstractButton* button, bool checked);

private:
    PushButton* m_consoleButton;
    PushButton* m_issuesButton;
    ButtonFlasher* m_consoleFlasher;
    ButtonFlasher* m_issuesFlasher;
};

#endif // OUTPUTBAR_H