#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QWidget>

class QStackedWidget;
class AndroidPlatformWidget;

class BuildsPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    QStackedWidget* stackedWidget() const;
    AndroidPlatformWidget* androidWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QStackedWidget* m_stackedWidget;
    AndroidPlatformWidget* m_androidWidget;
};

#endif // BUILDSPANE_H
