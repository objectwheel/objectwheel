#ifndef DEVICESBUTTON_H
#define DEVICESBUTTON_H

#include <QPushButton>

class QActionGroup;

class DevicesButton : public QPushButton
{
    Q_OBJECT

public:
    explicit DevicesButton(QWidget* parent = nullptr);
    QList<QAction*> deviceActions() const;
    void addDeviceAction(QAction* action);
    void addDeviceActions(const QList<QAction*>& actions);
    void insertDeviceAction(QAction* before, QAction* action);
    void insertDeviceActions(QAction* before, const QList<QAction*>& actions);
    void removeDeviceAction(QAction* action);
    void setCurrentDeviceAction(QAction* action);

public slots:
    void onCurrentDeviceActionChange(QAction*);

private:
    void paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override;

signals:
    void currentDeviceActionChanged(QAction*);
    void currentDeviceActionTriggered(QAction*);

private:
    QPixmap m_devicesPixmap;
    QMenu* m_menu;
    QActionGroup* m_actionGroup;
    QAction* m_myComputerAction;
};

#endif // DEVICESBUTTON_H