#include <qmlcodeeditorwidget.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <fileexplorer.h>

#include <QToolBar>
#include <QToolButton>
#include <QSplitter>
#include <QLayout>

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget *parent) : QWidget(parent)
  , m_splitter(new QSplitter(this))
  , m_codeEditor(new QmlCodeEditor(this))
  , m_fileExplorer(new FileExplorer(this))
{
    m_splitter->addWidget(m_codeEditor);
    m_splitter->addWidget(m_fileExplorer);
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
    m_splitter->setHandleWidth(0);

    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_splitter);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
}

void QmlCodeEditorWidget::sweep()
{
    m_codeEditor->sweep();
    m_fileExplorer->sweep();
    // FIXME
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