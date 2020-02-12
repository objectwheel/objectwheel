#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QScrollArea>

class QLabel;
class ButtonSlice;
class StackedLayout;
class PlatformSelectionWidget;
class AndroidPlatformWidget;
class AndroidPlatformController;

class BuildsPane final : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    enum Buttons { Next, Back };

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    QLabel* platformLabel() const;
    ButtonSlice* buttonSlice() const;
    StackedLayout* stackedLayout() const;
    PlatformSelectionWidget* platformSelectionWidget() const;
    AndroidPlatformWidget* androidPlatformWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void charge() const;
    void discharge() const;

private:
    QLabel* m_platformLabel;
    ButtonSlice* m_buttonSlice;
    StackedLayout* m_stackedLayout;
    PlatformSelectionWidget* m_platformSelectionWidget;
    AndroidPlatformWidget* m_androidPlatformWidget;
    AndroidPlatformController* m_androidPlatformController;
};

#endif // BUILDSPANE_H
