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
#include "behaviorsettings.h"
#include "behaviorsettingspage.h"
#include "completionsettings.h"
//#include "marginsettings.h"
//#include "displaysettings.h"
//#include "displaysettingspage.h"
#include "typingsettings.h"
#include "storagesettings.h"
#include "tabsettings.h"
//#include "extraencodingsettings.h"
#include "icodestylepreferences.h"
//#include "icodestylepreferencesfactory.h"
#include "completionsettingspage.h"
//#include <texteditor/generichighlighter/highlightersettingspage.h>
//#include <texteditor/snippets/snippetssettingspage.h>

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
    BehaviorSettingsPage *m_behaviorSettingsPage;
//    DisplaySettingsPage *m_displaySettingsPage;
//    HighlighterSettingsPage *m_highlighterSettingsPage;
//    SnippetsSettingsPage *m_snippetsSettingsPage;
    CompletionSettingsPage *m_completionSettingsPage;

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

    // Add the GUI used to configure the tab, storage and interaction settings
    BehaviorSettingsPageParameters behaviorSettingsPageParameters;
    behaviorSettingsPageParameters.id = Constants::TEXT_EDITOR_BEHAVIOR_SETTINGS;
    behaviorSettingsPageParameters.displayName = tr("Behavior");
    behaviorSettingsPageParameters.settingsPrefix = QLatin1String("text");
    d->m_behaviorSettingsPage = new BehaviorSettingsPage(behaviorSettingsPageParameters, this);

//    DisplaySettingsPageParameters displaySettingsPageParameters;
//    displaySettingsPageParameters.id = Constants::TEXT_EDITOR_DISPLAY_SETTINGS;
//    displaySettingsPageParameters.displayName = tr("Display");
//    displaySettingsPageParameters.settingsPrefix = QLatin1String("text");
//    d->m_displaySettingsPage = new DisplaySettingsPage(displaySettingsPageParameters, this);

//    d->m_highlighterSettingsPage =
//        new HighlighterSettingsPage(Constants::TEXT_EDITOR_HIGHLIGHTER_SETTINGS, this);
//    d->m_snippetsSettingsPage =
//        new SnippetsSettingsPage(Constants::TEXT_EDITOR_SNIPPETS_SETTINGS, this);
    d->m_completionSettingsPage = new CompletionSettingsPage(this);

    connect(d->m_behaviorSettingsPage, &BehaviorSettingsPage::typingSettingsChanged,
            this, &TextEditorSettings::typingSettingsChanged);
    connect(d->m_behaviorSettingsPage, &BehaviorSettingsPage::storageSettingsChanged,
            this, &TextEditorSettings::storageSettingsChanged);
    connect(d->m_behaviorSettingsPage, &BehaviorSettingsPage::behaviorSettingsChanged,
            this, &TextEditorSettings::behaviorSettingsChanged);
//    connect(d->m_behaviorSettingsPage, &BehaviorSettingsPage::extraEncodingSettingsChanged,
//            this, &TextEditorSettings::extraEncodingSettingsChanged);
//    connect(d->m_displaySettingsPage, &DisplaySettingsPage::marginSettingsChanged,
//            this, &TextEditorSettings::marginSettingsChanged);
//    connect(d->m_displaySettingsPage, &DisplaySettingsPage::displaySettingsChanged,
//            this, &TextEditorSettings::displaySettingsChanged);
    connect(d->m_completionSettingsPage, &CompletionSettingsPage::completionSettingsChanged,
            this, &TextEditorSettings::completionSettingsChanged);
//    connect(d->m_completionSettingsPage, &CompletionSettingsPage::commentsSettingsChanged,
//            this, &TextEditorSettings::commentsSettingsChanged);
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

const TypingSettings &TextEditorSettings::typingSettings()
{
    return d->m_behaviorSettingsPage->typingSettings();
}

const StorageSettings &TextEditorSettings::storageSettings()
{
    return d->m_behaviorSettingsPage->storageSettings();
}

const BehaviorSettings &TextEditorSettings::behaviorSettings()
{
    return d->m_behaviorSettingsPage->behaviorSettings();
}

//const MarginSettings &TextEditorSettings::marginSettings()
//{
//    return d->m_displaySettingsPage->marginSettings();
//}

//const DisplaySettings &TextEditorSettings::displaySettings()
//{
//    return d->m_displaySettingsPage->displaySettings();
//}

const CompletionSettings &TextEditorSettings::completionSettings()
{
    return d->m_completionSettingsPage->completionSettings();
}

//const HighlighterSettings &TextEditorSettings::highlighterSettings()
//{
//    return d->m_highlighterSettingsPage->highlighterSettings();
//}

//const ExtraEncodingSettings &TextEditorSettings::extraEncodingSettings()
//{
//    return d->m_behaviorSettingsPage->extraEncodingSettings();
//}

//const CommentsSettings &TextEditorSettings::commentsSettings()
//{
//    return d->m_completionSettingsPage->commentsSettings();
//}

//void TextEditorSettings::registerCodeStyleFactory(ICodeStylePreferencesFactory *factory)
//{
//    d->m_languageToFactory.insert(factory->languageId(), factory);
//}

//void TextEditorSettings::unregisterCodeStyleFactory(Core::Id languageId)
//{
//    d->m_languageToFactory.remove(languageId);
//}

//QMap<Core::Id, ICodeStylePreferencesFactory *> TextEditorSettings::codeStyleFactories()
//{
//    return d->m_languageToFactory;
//}

//ICodeStylePreferencesFactory *TextEditorSettings::codeStyleFactory(Core::Id languageId)
//{
//    return d->m_languageToFactory.value(languageId);
//}

ICodeStylePreferences *TextEditorSettings::codeStyle()
{
    return d->m_behaviorSettingsPage->codeStyle();
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

//CodeStylePool *TextEditorSettings::codeStylePool()
//{
//    return d->m_behaviorSettingsPage->codeStylePool();
//}

//CodeStylePool *TextEditorSettings::codeStylePool(Core::Id languageId)
//{
//    return d->m_languageToCodeStylePool.value(languageId);
//}

//void TextEditorSettings::registerCodeStylePool(Core::Id languageId, CodeStylePool *pool)
//{
//    d->m_languageToCodeStylePool.insert(languageId, pool);
//}

//void TextEditorSettings::unregisterCodeStylePool(Core::Id languageId)
//{
//    d->m_languageToCodeStylePool.remove(languageId);
//}

void TextEditorSettings::registerMimeTypeForLanguageId(const char *mimeType, Core::Id languageId)
{
    d->m_mimeTypeToLanguage.insert(QString::fromLatin1(mimeType), languageId);
}

Core::Id TextEditorSettings::languageId(const QString &mimeType)
{
    return d->m_mimeTypeToLanguage.value(mimeType);
}
