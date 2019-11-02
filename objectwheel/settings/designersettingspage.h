#ifndef DESIGNERSETTINGSPAGE_H
#define DESIGNERSETTINGSPAGE_H

#include <settingspage.h>

class DesignerSettingsPage final : public SettingsPage
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerSettingsPage)

public:
    explicit DesignerSettingsPage(QWidget* parent = nullptr);
    QIcon icon() const override;
};

#endif // DESIGNERSETTINGSPAGE_H