#ifndef BOTTOMBAR_H
#define BOTTOMBAR_H

#include <QWidget>

class QHBoxLayout;
class QAbstractButton;
class PushButton;

class BottomBar : public QWidget
{
    Q_OBJECT
public:
    explicit BottomBar(QWidget* parent = nullptr);
    QAbstractButton* consoleButton() const;
    QAbstractButton* issuesButton() const;

public:
    void flash(QAbstractButton*);

public slots:
    void sweep();

protected:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void buttonActivated(QAbstractButton* button, bool checked);

private:
    QHBoxLayout* m_layout;
    PushButton* m_consoleButton;
    PushButton* m_issuesButton;
};

#endif // BOTTOMBAR_H