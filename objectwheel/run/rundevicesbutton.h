#ifndef RUNDEVICESBUTTON_H
#define RUNDEVICESBUTTON_H

#include <QPushButton>
#include <tooltipwidget.h>

class RunDevicesButton final : public ToolTipWidget<QPushButton>
{
    Q_OBJECT
    Q_DISABLE_COPY(RunDevicesButton)

    enum {
        SPACING = 3,
        LEFT_PADDING = 8,
        FORWARD_ARROW_LENGTH = 9,
        RIGHT_PADDING = LEFT_PADDING + 2
    };

    using DeviceInfo = QVariantMap;

public:
    explicit RunDevicesButton(QWidget* parent = nullptr);

    bool hasDevice(const QString& uid) const;
    void addDevice(const DeviceInfo& deviceInfo);
    void removeDevice(const QString& uid);

    QString currentDevice() const;
    void setCurrentDevice(const QString& uid);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void paintEvent(QPaintEvent*) override;

private:
    QMenu* m_menu;
};

#endif // RUNDEVICESBUTTON_H