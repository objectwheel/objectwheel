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
    void discharge();
    void flash(QAbstractButton*);

private slots:
    void setLeftShowHideButtonToolTip(bool);
    void setRightShowHideButtonToolTip(bool);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

signals:
    void showHideLeftPanesButtonActivated(bool);
    void showHideRightPanesButtonActivated(bool);
    void buttonActivated(QAbstractButton* button, bool checked);

private:
    QHBoxLayout* m_layout;
    PushButton* m_consoleButton;
    PushButton* m_issuesButton;
    PushButton* m_showHideLeftPanesButton;
    PushButton* m_showHideRightPanesButton;
};

#endif // BOTTOMBAR_H