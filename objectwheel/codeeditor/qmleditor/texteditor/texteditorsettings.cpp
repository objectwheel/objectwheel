/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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

#include "texteditorsettings.h"

#include <qmlcodeeditor.h>
//#include "behaviorsettings.h"
#include "completionsettings.h"
#include "simplecodestylepreferences.h"
//#include "marginsettings.h"
//#include "displaysettings.h"
#include "typingsettings.h"
#include "storagesettings.h"
#include "tabsettings.h"
//#include "extraencodingsettings.h"
#include "icodestylepreferences.h"
//#include "icodestylepreferencesfactory.h"
//#include <extensionsystem/pluginmanager.h>
//#include <coreplugin/icore.h>
#include <utils/qtcassert.h>

#include <QApplication>

using namespace TextEditor;
using namespace TextEditor::Constants;
using namespace TextEditor::Internal;

namespace TextEditor {
namespace Internal {

class TextEditorSettingsPrivate
{
public:
//    QMap<Core::Id, ICodeStylePreferencesFactory *> m_languageToFactory;

    QMap<Core::Id, ICodeStylePreferences *> m_languageToCodeStyle;
//    QMap<Core::Id, CodeStylePool *> m_languageToCodeStylePool;
    QMap<QString, Core::Id> m_mimeTypeToLanguage;
};

} // namespace Internal
} // namespace TextEditor


static TextEditorSettingsPrivate *d = 0;
static TextEditorSettings *m_instance = 0;

TextEditorSettings::TextEditorSettings()
{
    QTC_ASSERT(!m_instance, return);
    m_instance = this;
    d = new Internal::TextEditorSettingsPrivate;
}

TextEditorSettings::~TextEditorSettings()
{
    delete d;

    m_instance = 0;
}

TextEditorSettings *TextEditorSettings::instance()
{
    return m_instance;
}

const CompletionSettings TextEditorSettings::completionSettings()
{
    return CompletionSettings();
}

ICodeStylePreferences *TextEditorSettings::codeStyle()
{
    static SimpleCodeStylePreferences* m_codeStyle = new SimpleCodeStylePreferences;
    m_codeStyle->setDisplayName(tr("Global", "Settings"));
    m_codeStyle->setId(Constants::GLOBAL_SETTINGS_ID);
    return m_codeStyle;
}

ICodeStylePreferences *TextEditorSettings::codeStyle(Core::Id languageId)
{
    return d->m_languageToCodeStyle.value(languageId, codeStyle());
}

QMap<Core::Id, ICodeStylePreferences *> TextEditorSettings::codeStyles()
{
    return d->m_languageToCodeStyle;
}

void TextEditorSettings::registerCodeStyle(Core::Id languageId, ICodeStylePreferences *prefs)
{
    d->m_languageToCodeStyle.insert(languageId, prefs);
}

void TextEditorSettings::unregisterCodeStyle(Core::Id languageId)
{
    d->m_languageToCodeStyle.remove(languageId);
}

void TextEditorSettings::registerMimeTypeForLanguageId(const char *mimeType, Core::Id languageId)
{
    d->m_mimeTypeToLanguage.insert(QString::fromLatin1(mimeType), languageId);
}

Core::Id TextEditorSettings::languageId(const QString &mimeType)
{
    return d->m_mimeTypeToLanguage.value(mimeType);
}
