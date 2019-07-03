#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <settingspage.h>

class GeneralSettingsPage final : public SettingsPage
{
    Q_OBJECT
    Q_DISABLE_COPY(GeneralSettingsPage)

public:
    explicit GeneralSettingsPage(QWidget* parent = nullptr);
    QIcon icon() const override;
};

#endif // GENERALSETTINGSPAGE_H