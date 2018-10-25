#ifndef CODEEDITORSETTINGSPAGE_H
#define CODEEDITORSETTINGSPAGE_H

#include <settingspage.h>

class CodeEditorSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit CodeEditorSettingsPage(QWidget* parent = nullptr);
    bool containsWord(const QString &word) const override;
    QIcon icon() const override;
};

#endif // CODEEDITORSETTINGSPAGE_H