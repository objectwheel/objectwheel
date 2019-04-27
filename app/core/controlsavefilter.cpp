#include <controlsavefilter.h>
#include <saveutils.h>
#include <parserutils.h>
#include <control.h>
#include <controlpropertymanager.h>
#include <controlpreviewingmanager.h>
#include <qmlcodedocument.h>

#define internal(x) static_cast<QmlCodeEditorWidget::InternalDocument*>((x))
#define internalDir(x) SaveUtils::toThisDir(internal((x))->control->dir())
#define fullPath(x) internalDir((x)) + separator() + internal((x))->relativePath

ControlSaveFilter::ControlSaveFilter(QObject* parent) : QObject(parent)
{
}

void ControlSaveFilter::beforeSave(QmlCodeEditorWidget::Document* document)
{
    if (document->scope != QmlCodeEditorToolBar::Internal)
        return;

    if (internal(document)->relativePath != SaveUtils::mainQmlFile())
        return;

    Control* control = internal(document)->control;
    QmlCodeDocument* doc = internal(document)->document;

    m_id = ParserUtils::property(doc, fullPath(document), "id");

    if (m_id.isEmpty()) {
        ParserUtils::setProperty(doc, fullPath(document), "id", control->id());
        m_id = control->id();
    }
}

void ControlSaveFilter::afterSave(QmlCodeEditorWidget::Document* document)
{
    if (document->scope != QmlCodeEditorToolBar::Internal)
        return;

    Control* control = internal(document)->control;

    if (internal(document)->relativePath == SaveUtils::mainQmlFile()) {
        if (control->id() != m_id)
            ControlPropertyManager::setId(control, m_id, ControlPropertyManager::SaveChanges); // For refactorId
    }

    if (control->form())
        ControlPreviewingManager::scheduleFormCodeUpdate(control->uid());
    else
        ControlPreviewingManager::scheduleControlCodeUpdate(control->uid());
}
