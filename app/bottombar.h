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
    QAbstractButton* activeButton() const;
    QAbstractButton* consoleButton() const;
    QAbstractButton* issuesButton() const;

public slots:
    void sweep();
    void flash(QAbstractButton*);

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
    PushButton* m_hideShowLeftPanesButton;
    PushButton* m_hideShowRightPanesButton;
};

#endif // BOTTOMBAR_H