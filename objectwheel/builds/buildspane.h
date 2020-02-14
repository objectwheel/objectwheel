#ifndef BUILDSPANE_H
#define BUILDSPANE_H

#include <QScrollArea>

class QLabel;
class ButtonSlice;
class StackedLayout;
class DownloadWidget;
class PlatformSelectionWidget;
class AndroidPlatformWidget;

class BuildsPane final : public QScrollArea
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildsPane)

public:
    enum Buttons { Next, Back };

public:
    explicit BuildsPane(QWidget* parent = nullptr);

    QLabel* platformLabel() const;
    QLabel* iconLabel() const;
    QLabel* descriptionLabel() const;
    ButtonSlice* buttonSlice() const;
    StackedLayout* stackedLayout() const;
    DownloadWidget* downloadWidget() const;
    PlatformSelectionWidget* platformSelectionWidget() const;
    AndroidPlatformWidget* androidPlatformWidget() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QLabel* m_iconLabel;
    QLabel* m_platformLabel;
    QLabel* m_descriptionLabel;
    ButtonSlice* m_buttonSlice;
    StackedLayout* m_stackedLayout;
    DownloadWidget* m_downloadWidget;
    PlatformSelectionWidget* m_platformSelectionWidget;
    AndroidPlatformWidget* m_androidPlatformWidget;
};

#endif // BUILDSPANE_H
