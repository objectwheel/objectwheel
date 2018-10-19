#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <settingspage.h>

class GeneralSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit GeneralSettingsPage(QWidget* parent = nullptr);

    QString title() const override;
    QIcon icon() const override;
    bool containsWord(const QString &word) const override;
};

#endif // GENERALSETTINGSPAGE_H