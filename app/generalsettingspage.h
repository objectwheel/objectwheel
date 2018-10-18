#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <settingspage.h>

class GeneralSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit GeneralSettingsPage(QWidget* parent = nullptr);

};

#endif // GENERALSETTINGSPAGE_H