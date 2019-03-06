#ifndef DEVICESBUTTON_H
#define DEVICESBUTTON_H

#include <QPushButton>

class QActionGroup;

class DevicesButton : public QPushButton
{
    Q_OBJECT

public:
    explicit DevicesButton(QWidget* parent = nullptr);

public slots:
    void addDevice(const QVariantMap& deviceInfo);
    void removeDevice(const QString& uid);
    void setActiveDevice(const QString& uid);
    QString activeDevice() const;

public:
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private slots:
    void onMenuItemActivation(QAction* action);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QSize recomputeSizeHint() const;
    bool deviceExists(const QString& uid) const;

signals:
    void activeDeviceChanged(const QString& uid);
    void activeDeviceTriggered(const QString& uid);

private:
    const QIcon m_devicesIcon;
    QMenu* m_menu;
    QActionGroup* m_actionGroup;
};

#endif // DEVICESBUTTON_H