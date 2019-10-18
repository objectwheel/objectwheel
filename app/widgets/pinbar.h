#ifndef PINBAR_H
#define PINBAR_H

#include <QWidget>
#include <QIcon>

class QLabel;
class QToolButton;
class QDockWidget;

class PinBar final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PinBar)

public:
    explicit PinBar(QDockWidget* dockWidget);

    QIcon icon() const;
    void setIcon(const QIcon& icon);

    QString title() const;
    void setTitle(const QString& title);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onDetachButtonClick();
    void onDockWidgetTopLevelChange();

private:
    void paintEvent(QPaintEvent* event) override;

private:
    QIcon m_icon;
    QString m_title;
    QDockWidget* m_dockWidget;
    QLabel* m_iconLabel;
    QLabel* m_titleLabel;
    QToolButton* m_detachButton;
    QToolButton* m_closeButton;
};

#endif // PINBAR_H
