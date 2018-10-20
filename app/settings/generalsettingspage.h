#ifndef GENERALSETTINGSPAGE_H
#define GENERALSETTINGSPAGE_H

#include <settingspage.h>

class GeneralSettingsPage : public SettingsPage
{
    Q_OBJECT

public:
    explicit GeneralSettingsPage(QWidget* parent = nullptr);
    void clean() override;
    void apply() override;
    QIcon icon() const override;
    QString title() const override;
    bool containsWord(const QString &word) const override;
};

#endif // GENERALSETTINGSPAGE_H