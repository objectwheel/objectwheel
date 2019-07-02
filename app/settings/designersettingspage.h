#ifndef DESIGNERSETTINGSPAGE_H
#define DESIGNERSETTINGSPAGE_H

#include <settingspage.h>

class DesignerSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit DesignerSettingsPage(QWidget* parent = nullptr);
    QIcon icon() const override;
};

#endif // DESIGNERSETTINGSPAGE_H