#include <qmlcodeeditorwidget.h>
#include <filemanager.h>

#include <QToolBar>
#include <QToolButton>

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget *parent) : QWidget(parent)
  , m_toolbar(new QToolBar)
  , m_pinButton(new QToolButton)
  , m_undoButton(new QToolButton)
  , m_redoButton(new QToolButton)
  , m_closeButton(new QToolButton)
  , m_saveButton(new QToolButton)
  , m_cutButton(new QToolButton)
  , m_copyButton(new QToolButton)
  , m_pasteButton(new QToolButton)
{
}

void QmlCodeEditorWidget::openDocument(Control* control)
{
    Q_ASSERT(control);

    for (InternalDocument* document : m_internalDocuments) {
        if (document->control == control)
            return openDocument(document);
    }

    openDocument(createNewDocument(control));
}

void QmlCodeEditorWidget::openDocument(const QString& path, QmlCodeEditorWidget::DocumentType type)
{
    Q_ASSERT(type != Internal);
    Q_ASSERT(!path.isEmpty());
    Q_ASSERT(exists(path));

    if (type == Regular) {
        for (ExternalDocument* document : m_regularDocuments) {
            if (document->path == path)
                return openDocument(document);
        }
    } else {
        for (ExternalDocument* document : m_globalDocuments) {
            if (document->path == path)
                return openDocument(document);
        }
    }

    openDocument(createNewDocument(path, type));
}

void QmlCodeEditorWidget::openDocument(QmlCodeEditorWidget::Document* document) const
{

}

QmlCodeEditorWidget::Document* QmlCodeEditorWidget::createNewDocument(
        const QString& path,
        QmlCodeEditorWidget::DocumentType type) const
{

}

QmlCodeEditorWidget::Document*QmlCodeEditorWidget::createNewDocument(Control* control) const
{

}