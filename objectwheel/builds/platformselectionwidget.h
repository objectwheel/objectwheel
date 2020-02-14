#ifndef PLATFORMSELECTIONWIDGET_H
#define PLATFORMSELECTIONWIDGET_H

#include <QWidget>

class QListWidget;
class ButtonSlice;

class PlatformSelectionWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PlatformSelectionWidget)

public:
    enum Buttons { Back, Next };
    enum Platform {
        Invalid,
        Android,
        iOS,
        macOS,
        Windows,
        Linux,
        RaspberryPi
    };
    Q_ENUM(Platform)

public:
    explicit PlatformSelectionWidget(QWidget* parent = nullptr);

    QListWidget* platformList() const;
    ButtonSlice* buttonSlice() const;
    Platform currentPlatform() const;

private:
    QListWidget* m_platformList;
    ButtonSlice* m_buttonSlice;
};

#endif // PLATFORMSELECTIONWIDGET_H
