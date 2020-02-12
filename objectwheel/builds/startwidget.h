#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QWidget>

class QListWidget;
class StartWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(StartWidget)

public:
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
    explicit StartWidget(QWidget* parent = nullptr);

    QListWidget* platformList() const;

    Platform currentPlatform() const;

private:
    QListWidget* m_platformList;
};

#endif // STARTWIDGET_H
