#include <controlsavefilter.h>
#include <saveutils.h>
#include <parserutils.h>
#include <control.h>
#include <controlpropertymanager.h>
#include <controlpreviewingmanager.h>
#include <qmlcodedocument.h>

#define designs(x) static_cast<QmlCodeEditorWidget::DesignsDocument*>((x))

ControlSaveFilter::ControlSaveFilter(QObject* parent) : QObject(parent)
{
}

void ControlSaveFilter::beforeSave(QmlCodeEditorWidget::Document* document)
{
    if (document->scope != QmlCodeEditorToolBar::Designs)
        return;

    if (designs(document)->relativePath != SaveUtils::controlMainQmlFileName())
        return;

    Control* control = designs(document)->control;
    QmlCodeDocument* doc = designs(document)->document;

    m_id = ParserUtils::property(doc, control->dir(), "id");

    if (m_id.isEmpty()) {
        ParserUtils::setProperty(doc, control->dir(), "id", control->id());
        m_id = control->id();
    }
}

void ControlSaveFilter::afterSave(QmlCodeEditorWidget::Document* document)
{
    if (document->scope != QmlCodeEditorToolBar::Designs)
        return;

    Control* control = designs(document)->control;

    if (designs(document)->relativePath == SaveUtils::controlMainQmlFileName()) {
        if (control->id() != m_id)
            ControlPropertyManager::setId(control, m_id, ControlPropertyManager::SaveChanges); // For refactorId
    }

    if (control->form())
        ControlPreviewingManager::scheduleFormCodeUpdate(control->uid());
    else
        ControlPreviewingManager::scheduleControlCodeUpdate(control->uid());
}
