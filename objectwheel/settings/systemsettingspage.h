#ifndef SYSTEMSETTINGSPAGE_H
#define SYSTEMSETTINGSPAGE_H

#include <settingspage.h>

class SystemSettingsPage final : public SettingsPage
{
    Q_OBJECT
    Q_DISABLE_COPY(SystemSettingsPage)

public:
    explicit SystemSettingsPage(QWidget* parent = nullptr);
    QIcon icon() const override;
};

#endif // SYSTEMSETTINGSPAGE_H