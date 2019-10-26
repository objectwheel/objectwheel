#ifndef PINBAR_H
#define PINBAR_H

#include <QWidget>

class QLabel;
class QToolButton;
class QDockWidget;

class PinBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PinBar)

public:
    explicit PinBar(QDockWidget* dockWidget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onDetachButtonClick();
    void onDockWidgetTopLevelChange();

private:
    void paintEvent(QPaintEvent* event) override;

signals:
    void dockWidgetHid(QDockWidget* dockWidget);

private:
    QDockWidget* m_dockWidget;
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QToolButton* m_detachButton;
    QToolButton* m_closeButton;
};

#endif // PINBAR_H
