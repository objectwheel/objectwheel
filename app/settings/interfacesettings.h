#ifndef INTERFACESETTINGS_H
#define INTERFACESETTINGS_H

#include <QWidget>

namespace Ui {
class InterfaceSettings;
}

class InterfaceSettings : public QWidget
{
    Q_OBJECT

public:
    explicit InterfaceSettings(QWidget *parent = 0);
    ~InterfaceSettings();

private:
    Ui::InterfaceSettings *ui;
};

#endif // INTERFACESETTINGS_H
