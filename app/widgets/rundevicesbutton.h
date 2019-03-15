#ifndef RUNDEVICESBUTTON_H
#define RUNDEVICESBUTTON_H

#include <QPushButton>

class RunDevicesButton final : public QPushButton
{
    Q_OBJECT

public:
    explicit RunDevicesButton(QWidget* parent = nullptr);

    QString activeDevice() const;
    void setActiveDevice(const QString& uid);

    bool hasDevice(const QString& uid) const;
    void addDevice(const QVariantMap& deviceInfo);
    void removeDevice(const QString& uid);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onTriggered(QAction* action);

private:
    void paintEvent(QPaintEvent*) override;

signals:
    void triggered(const QString& uid);

private:
    QMenu* m_menu;
};

#endif // RUNDEVICESBUTTON_H