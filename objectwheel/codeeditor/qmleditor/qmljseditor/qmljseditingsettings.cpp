// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

#include "qmljseditingsettings.h"
#include "qmljseditorconstants.h"
#include "qmljseditortr.h"

#include <qmljs/qmljscheck.h>
#include <qmljs/qmljsstaticanalysismessage.h>
#include <qmljstools/qmljstoolsconstants.h>
#include <utils/algorithm.h>
#include <utils/macroexpander.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QSettings>
#include <QTextStream>
#include <QTreeView>

const char AUTO_FORMAT_ON_SAVE[] = "QmlJSEditor.AutoFormatOnSave";
const char AUTO_FORMAT_ONLY_CURRENT_PROJECT[] = "QmlJSEditor.AutoFormatOnlyCurrentProject";
const char QML_CONTEXTPANE_KEY[] = "QmlJSEditor.ContextPaneEnabled";
const char QML_CONTEXTPANEPIN_KEY[] = "QmlJSEditor.ContextPanePinned";
const char FOLD_AUX_DATA[] = "QmlJSEditor.FoldAuxData";
const char UIQML_OPEN_MODE[] = "QmlJSEditor.openUiQmlMode";
const char FORMAT_COMMAND[] = "QmlJSEditor.formatCommand";
const char FORMAT_COMMAND_OPTIONS[] = "QmlJSEditor.formatCommandOptions";
const char CUSTOM_COMMAND[] = "QmlJSEditor.useCustomFormatCommand";
const char CUSTOM_ANALYZER[] = "QmlJSEditor.useCustomAnalyzer";
const char DISABLED_MESSAGES[] = "QmlJSEditor.disabledMessages";
const char DISABLED_MESSAGES_NONQUICKUI[] = "QmlJSEditor.disabledMessagesNonQuickUI";
const char DEFAULT_CUSTOM_FORMAT_COMMAND[] = "%{CurrentDocument:Project:QT_HOST_BINS}/qmlformat";

using namespace QmlJSEditor;

static QList<int> defaultDisabledMessages()
{
    static const QList<int> disabledByDefault = Utils::transform(
                QmlJS::Check::defaultDisabledMessages(),
                [](QmlJS::StaticAnalysis::Type t) { return int(t); });
    return disabledByDefault;
}

static QList<int> defaultDisabledMessagesNonQuickUi()
{
    static const QList<int> disabledForNonQuickUi = Utils::transform(
        QmlJS::Check::defaultDisabledMessagesForNonQuickUi(),
        [](QmlJS::StaticAnalysis::Type t){ return int(t); });
    return disabledForNonQuickUi;
}

void QmlJsEditingSettings::set()
{
    if (get() != *this)
        toSettings(Core::ICore::settings());
}

static QStringList intListToStringList(const QList<int> &list)
{
    return Utils::transform(list, [](int v) { return QString::number(v); });
}

QList<int> intListFromStringList(const QStringList &list)
{
    return Utils::transform<QList<int> >(list, [](const QString &v) { return v.toInt(); });
}

static QStringList defaultDisabledMessagesAsString()
{
    static const QStringList result = intListToStringList(defaultDisabledMessages());
    return result;
}

static QStringList defaultDisabledNonQuickUiAsString()
{
    static const QStringList result = intListToStringList(defaultDisabledMessagesNonQuickUi());
    return result;
}

void QmlJsEditingSettings::fromSettings(QSettings *settings)
{
    settings->beginGroup(QmlJSEditor::Constants::SETTINGS_CATEGORY_QML);
    m_enableContextPane = settings->value(QML_CONTEXTPANE_KEY, QVariant(false)).toBool();
    m_pinContextPane = settings->value(QML_CONTEXTPANEPIN_KEY, QVariant(false)).toBool();
    m_autoFormatOnSave = settings->value(AUTO_FORMAT_ON_SAVE, QVariant(false)).toBool();
    m_autoFormatOnlyCurrentProject
        = settings->value(AUTO_FORMAT_ONLY_CURRENT_PROJECT, QVariant(false)).toBool();
    m_foldAuxData = settings->value(FOLD_AUX_DATA, QVariant(true)).toBool();
    m_uiQmlOpenMode = settings->value(UIQML_OPEN_MODE, "").toString();
    m_formatCommand = settings->value(FORMAT_COMMAND, {}).toString();
    m_formatCommandOptions = settings->value(FORMAT_COMMAND_OPTIONS, {}).toString();
    m_useCustomFormatCommand = settings->value(CUSTOM_COMMAND, QVariant(false)).toBool();
    m_useCustomAnalyzer = settings->value(CUSTOM_ANALYZER, QVariant(false)).toBool();

    m_disabledMessages = Utils::toSet(
        intListFromStringList(settings->value(DISABLED_MESSAGES,
                              defaultDisabledMessagesAsString()).toStringList()));

    m_disabledMessagesForNonQuickUi = Utils::toSet(
        intListFromStringList(settings->value(DISABLED_MESSAGES_NONQUICKUI,
                              defaultDisabledNonQuickUiAsString()).toStringList()));

    settings->endGroup();
}

void QmlJsEditingSettings::toSettings(QSettings *settings) const
{
    settings->beginGroup(QmlJSEditor::Constants::SETTINGS_CATEGORY_QML);
    settings->setValue(QML_CONTEXTPANE_KEY, m_enableContextPane);
    settings->setValue(QML_CONTEXTPANEPIN_KEY, m_pinContextPane);
    settings->setValue(AUTO_FORMAT_ON_SAVE, m_autoFormatOnSave);
    settings->setValue(AUTO_FORMAT_ONLY_CURRENT_PROJECT, m_autoFormatOnlyCurrentProject);
    settings->setValue(FOLD_AUX_DATA, m_foldAuxData);
    settings->setValue(UIQML_OPEN_MODE, m_uiQmlOpenMode);
    Utils::QtcSettings::setValueWithDefault(settings, FORMAT_COMMAND, m_formatCommand, {});
    Utils::QtcSettings::setValueWithDefault(settings,
                                            FORMAT_COMMAND_OPTIONS,
                                            m_formatCommandOptions,
                                            {});
    Utils::QtcSettings::setValueWithDefault(settings,
                                            CUSTOM_COMMAND,
                                            m_useCustomFormatCommand,
                                            false);
    Utils::QtcSettings::setValueWithDefault(settings,
                                            CUSTOM_ANALYZER,
                                            m_useCustomAnalyzer,
                                            false);
    Utils::QtcSettings::setValueWithDefault(settings,
                                            DISABLED_MESSAGES,
                                            intListToStringList(Utils::sorted(Utils::toList(m_disabledMessages))),
                                            defaultDisabledMessagesAsString());
    Utils::QtcSettings::setValueWithDefault(settings,
                                            DISABLED_MESSAGES_NONQUICKUI,
                                            intListToStringList(Utils::sorted(Utils::toList(m_disabledMessagesForNonQuickUi))),
                                            defaultDisabledNonQuickUiAsString());
    settings->endGroup();
}

bool QmlJsEditingSettings::equals(const QmlJsEditingSettings &other) const
{
    return m_enableContextPane == other.m_enableContextPane
           && m_pinContextPane == other.m_pinContextPane
           && m_autoFormatOnSave == other.m_autoFormatOnSave
           && m_autoFormatOnlyCurrentProject == other.m_autoFormatOnlyCurrentProject
           && m_foldAuxData == other.m_foldAuxData
           && m_uiQmlOpenMode == other.m_uiQmlOpenMode && m_formatCommand == other.m_formatCommand
           && m_formatCommandOptions == other.m_formatCommandOptions
           && m_useCustomFormatCommand == other.m_useCustomFormatCommand
           && m_useCustomAnalyzer == other.m_useCustomAnalyzer
           && m_disabledMessages == other.m_disabledMessages
           && m_disabledMessagesForNonQuickUi == other.m_disabledMessagesForNonQuickUi;
}

bool QmlJsEditingSettings::enableContextPane() const
{
    return m_enableContextPane;
}

void QmlJsEditingSettings::setEnableContextPane(const bool enableContextPane)
{
    m_enableContextPane = enableContextPane;
}

bool QmlJsEditingSettings::pinContextPane() const
{
    return m_pinContextPane;
}

void QmlJsEditingSettings::setPinContextPane(const bool pinContextPane)
{
    m_pinContextPane = pinContextPane;
}

bool QmlJsEditingSettings::autoFormatOnSave() const
{
    return m_autoFormatOnSave;
}

void QmlJsEditingSettings::setAutoFormatOnSave(const bool autoFormatOnSave)
{
    m_autoFormatOnSave = autoFormatOnSave;
}

bool QmlJsEditingSettings::autoFormatOnlyCurrentProject() const
{
    return m_autoFormatOnlyCurrentProject;
}

void QmlJsEditingSettings::setAutoFormatOnlyCurrentProject(const bool autoFormatOnlyCurrentProject)
{
    m_autoFormatOnlyCurrentProject = autoFormatOnlyCurrentProject;
}

bool QmlJsEditingSettings::foldAuxData() const
{
    return m_foldAuxData;
}

void QmlJsEditingSettings::setFoldAuxData(const bool foldAuxData)
{
    m_foldAuxData = foldAuxData;
}

QString QmlJsEditingSettings::defaultFormatCommand() const
{
    return DEFAULT_CUSTOM_FORMAT_COMMAND;
}

QString QmlJsEditingSettings::formatCommand() const
{
    return m_formatCommand;
}

void QmlJsEditingSettings::setFormatCommand(const QString &formatCommand)
{
    m_formatCommand = formatCommand;
}

QString QmlJsEditingSettings::formatCommandOptions() const
{
    return m_formatCommandOptions;
}

void QmlJsEditingSettings::setFormatCommandOptions(const QString &formatCommandOptions)
{
    m_formatCommandOptions = formatCommandOptions;
}

bool QmlJsEditingSettings::useCustomFormatCommand() const
{
    return m_useCustomFormatCommand;
}

void QmlJsEditingSettings::setUseCustomFormatCommand(bool customCommand)
{
    m_useCustomFormatCommand = customCommand;
}

const QString QmlJsEditingSettings::uiQmlOpenMode() const
{
    return m_uiQmlOpenMode;
}

void QmlJsEditingSettings::setUiQmlOpenMode(const QString &mode)
{
    m_uiQmlOpenMode = mode;
}

bool QmlJsEditingSettings::useCustomAnalyzer() const
{
    return m_useCustomAnalyzer;
}

void QmlJsEditingSettings::setUseCustomAnalyzer(bool customAnalyzer)
{
    m_useCustomAnalyzer = customAnalyzer;
}

QSet<int> QmlJsEditingSettings::disabledMessages() const
{
    return m_disabledMessages;
}

void QmlJsEditingSettings::setDisabledMessages(const QSet<int> &disabled)
{
    m_disabledMessages = disabled;
}

QSet<int> QmlJsEditingSettings::disabledMessagesForNonQuickUi() const
{
    return m_disabledMessagesForNonQuickUi;
}

void QmlJsEditingSettings::setDisabledMessagesForNonQuickUi(const QSet<int> &disabled)
{
    m_disabledMessagesForNonQuickUi = disabled;
}

QmlJsEditingSettings QmlJsEditingSettings::get()
{
    QmlJsEditingSettings settings;
    settings.fromSettings(Core::ICore::settings());
    return settings;
}
