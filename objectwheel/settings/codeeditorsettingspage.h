#ifndef CODEEDITORSETTINGSPAGE_H
#define CODEEDITORSETTINGSPAGE_H

#include <settingspage.h>

class CodeEditorSettingsPage final : public SettingsPage
{
    Q_OBJECT
    Q_DISABLE_COPY(CodeEditorSettingsPage)

public:
    explicit CodeEditorSettingsPage(QWidget* parent = nullptr);
    QIcon icon() const override;
};

#endif // CODEEDITORSETTINGSPAGE_H