#include "qmljseditingsettings.h"
#include "qmljseditorconstants.h"

#include <applicationcore.h>
#include <qmljstools/qmljstoolsconstants.h>

#include <QSettings>

using namespace QmlJSEditor;

QmlJsEditingSettings::QmlJsEditingSettings()
    : m_enableContextPane(false),
      m_pinContextPane(false),
      m_autoFormatOnSave(false),
      m_autoFormatOnlyCurrentProject(false)
{}

void QmlJsEditingSettings::set()
{
    if (get() != *this)
        toSettings(ApplicationCore::settings());
}

void QmlJsEditingSettings::fromSettings(QSettings *settings)
{
    settings->beginGroup(QLatin1String(QmlJSEditor::Constants::SETTINGS_CATEGORY_QML));
    m_enableContextPane = settings->value(
                QLatin1String(QmlJSEditor::Constants::QML_CONTEXTPANE_KEY),
                QVariant(false)).toBool();
    m_pinContextPane = settings->value(
                QLatin1String(QmlJSEditor::Constants::QML_CONTEXTPANEPIN_KEY),
                QVariant(false)).toBool();
    m_autoFormatOnSave = settings->value(
                QLatin1String(QmlJSEditor::Constants::AUTO_FORMAT_ON_SAVE),
                QVariant(false)).toBool();
    m_autoFormatOnlyCurrentProject = settings->value(
                QLatin1String(QmlJSEditor::Constants::AUTO_FORMAT_ONLY_CURRENT_PROJECT),
                QVariant(false)).toBool();
    settings->endGroup();
}

void QmlJsEditingSettings::toSettings(QSettings *settings) const
{
    settings->beginGroup(QLatin1String(QmlJSEditor::Constants::SETTINGS_CATEGORY_QML));
    settings->setValue(QLatin1String(QmlJSEditor::Constants::QML_CONTEXTPANE_KEY),
                       m_enableContextPane);
    settings->setValue(QLatin1String(QmlJSEditor::Constants::QML_CONTEXTPANEPIN_KEY),
                       m_pinContextPane);
    settings->setValue(QLatin1String(QmlJSEditor::Constants::AUTO_FORMAT_ON_SAVE),
                       m_autoFormatOnSave);
    settings->setValue(QLatin1String(QmlJSEditor::Constants::AUTO_FORMAT_ONLY_CURRENT_PROJECT),
                       m_autoFormatOnlyCurrentProject);
    settings->endGroup();
}

bool QmlJsEditingSettings::equals(const QmlJsEditingSettings &other) const
{
    return  m_enableContextPane == other.m_enableContextPane
            && m_pinContextPane == other.m_pinContextPane
            && m_autoFormatOnSave == other.m_autoFormatOnSave
            && m_autoFormatOnlyCurrentProject == other.m_autoFormatOnlyCurrentProject;
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

QmlJsEditingSettings QmlJsEditingSettings::get()
{
    return QmlJsEditingSettings();
}
