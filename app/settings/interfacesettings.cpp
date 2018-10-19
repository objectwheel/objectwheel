#include "interfacesettings.h"
#include "ui_interfacesettings.h"

InterfaceSettings::InterfaceSettings(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InterfaceSettings)
{
    ui->setupUi(this);
}

InterfaceSettings::~InterfaceSettings()
{
    delete ui;
}
