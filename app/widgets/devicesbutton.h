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

private slots:
    void onCurrentDeviceActionChange(QAction*);

protected:
    void paintEvent(QPaintEvent*) override;
    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    QSize recomputeSizeHint() const;

signals:
    void currentDeviceActionChanged(QAction*);
    void currentDeviceActionTriggered(QAction*);

private:
    const QIcon m_devicesIcon;
    QMenu* m_menu;
    QActionGroup* m_actionGroup;
    QAction* m_myComputerAction;
};

#endif // DEVICESBUTTON_H