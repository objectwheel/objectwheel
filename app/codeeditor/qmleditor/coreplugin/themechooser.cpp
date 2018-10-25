/****************************************************************************
**
** Copyright (C) 2016 Thorben Kroeger <thorbenkroeger@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "coreconstants.h"
//#include "icore.h"
//#include "manhattanstyle.h"
#include "themechooser.h"
#include <windowmanager.h>
#include <applicationcore.h>
#include <mainwindow.h>

#include <utils/algorithm.h>
#include <utils/theme/theme.h>
#include <utils/theme/theme_p.h>

#include <QAbstractListModel>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QSpacerItem>

using namespace Utils;

static const char themeNameKey[] = "ThemeName";

namespace Core {
namespace Internal {

ThemeEntry::ThemeEntry(Id id, const QString &filePath)
    : m_id(id)
    , m_filePath(filePath)
{
}

Id ThemeEntry::id() const
{
    return m_id;
}

QString ThemeEntry::displayName() const
{
    if (m_displayName.isEmpty() && !m_filePath.isEmpty()) {
        QSettings settings(m_filePath, QSettings::IniFormat);
        m_displayName = settings.value(QLatin1String(themeNameKey),
                                       QCoreApplication::tr("unnamed")).toString();
    }
    return m_displayName;
}

QString ThemeEntry::filePath() const
{
    return m_filePath;
}

static void addThemesFromPath(const QString &path, QList<ThemeEntry> *themes)
{
    static const QLatin1String extension("*.creatortheme");
    QDir themeDir(path);
    themeDir.setNameFilters({extension});
    themeDir.setFilter(QDir::Files);
    const QStringList themeList = themeDir.entryList();
    foreach (const QString &fileName, themeList) {
        QString id = QFileInfo(fileName).completeBaseName();
        themes->append(ThemeEntry(Id::fromString(id), themeDir.absoluteFilePath(fileName)));
    }
}

QList<ThemeEntry> ThemeEntry::availableThemes()
{
    QList<ThemeEntry> themes;

    static const QString installThemeDir = ApplicationCore::resourcePath() + QLatin1String("/themes");
    static const QString userThemeDir = ApplicationCore::userResourcePath() + QLatin1String("/themes");
    addThemesFromPath(installThemeDir, &themes);
    if (themes.isEmpty())
        qWarning() << "Warning: No themes found in installation: "
                   << QDir::toNativeSeparators(installThemeDir);
    // move default theme to front
    int defaultIndex = Utils::indexOf(themes, Utils::equal(&ThemeEntry::id, Id(Constants::DEFAULT_THEME)));
    if (defaultIndex > 0) { // == exists and not at front
        ThemeEntry defaultEntry = themes.takeAt(defaultIndex);
        themes.prepend(defaultEntry);
    }
    addThemesFromPath(userThemeDir, &themes);
    return themes;
}

Id ThemeEntry::themeSetting()
{
    const Id setting =
            Id::fromSetting(ApplicationCore::settings()->value(QLatin1String(Constants::SETTINGS_THEME),
                                                     QLatin1String(Constants::DEFAULT_THEME)));

    const QList<ThemeEntry> themes = availableThemes();
    if (themes.empty())
        return Id();
    const bool settingValid = Utils::contains(themes, Utils::equal(&ThemeEntry::id, setting));

    return settingValid ? setting : themes.first().id();
}

Theme *ThemeEntry::createTheme(Id id)
{
    if (!id.isValid())
        return nullptr;
    const ThemeEntry entry = Utils::findOrDefault(availableThemes(),
                                                  Utils::equal(&ThemeEntry::id, id));
    if (!entry.id().isValid())
        return nullptr;
    QSettings themeSettings(entry.filePath(), QSettings::IniFormat);
    Theme *theme = new Theme(entry.id().toString());
    theme->readSettings(themeSettings);
    return theme;
}

} // namespace Internal
} // namespace Core
