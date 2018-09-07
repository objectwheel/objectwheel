#include <qmlcodeeditorwidget.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <qmlcodeeditortoolbar.h>
#include <fileexplorer.h>
#include <qmlcodedocument.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <control.h>

#include <QApplication>
#include <QToolBar>
#include <QToolButton>
#include <QSplitter>
#include <QLayout>

// FIXME:
// What happens if a global open file get renamed
// What happens if a global open file get deleted
// What happens if a global open file get overwritten/content changed outside

// What happens if a internal open file get renamed
// What happens if a internal open file get deleted
// What happens if a internal open file get overwritten/content changed outside
// What happens if a control get deleted
// What happens if a control's dir changes
// What happens to the file explorer's root path if a control's dir changes

#define globalDir() SaveUtils::toGlobalDir(ProjectManager::dir())

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget *parent) : QWidget(parent)
  , m_splitter(new QSplitter(this))
  , m_codeEditor(new QmlCodeEditor(this))
  , m_fileExplorer(new FileExplorer(0))
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_splitter);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter->addWidget(m_codeEditor);
    m_splitter->addWidget(m_fileExplorer);
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
    m_splitter->setHandleWidth(0);

    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::saved,
            this, &QmlCodeEditorWidget::save);
    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::closed,
            this, &QmlCodeEditorWidget::close);
    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::showed,
            this, &QmlCodeEditorWidget::setFileExplorerVisible);
    connect(m_codeEditor->toolBar(), &QmlCodeEditorToolBar::pinned,
            this, &QmlCodeEditorWidget::pinned);
}

void QmlCodeEditorWidget::openGlobal(const QString& relativePath)
{
    GlobalDocument document;
    document.type = Global;
    document.document = nullptr;
    document.relativePath = QFileInfo(relativePath).canonicalFilePath();
//    document.
}

bool QmlCodeEditorWidget::globalExists(const QString& relativePath)
{
    for (GlobalDocument* document : m_globalDocuments) {
        if (document->relativePath == relativePath)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::internalExists(Control* control, const QString& relativePath)
{
    for (InternalDocument* document : m_internalDocuments) {
        if (document->relativePath == relativePath && control == document->control)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::externalExists(const QString& fullPath)
{
    for (ExternalDocument* document : m_externalDocuments) {
        if (document->fullPath == fullPath)
            return true;
    }
    return false;
}

QmlCodeEditorWidget::GlobalDocument* QmlCodeEditorWidget::getGlobal(const QString& relativePath)
{
    for (GlobalDocument* document : m_globalDocuments) {
        if (document->relativePath == relativePath)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::InternalDocument* QmlCodeEditorWidget::getInternal(Control* control, const QString& relativePath)
{
    for (InternalDocument* document : m_internalDocuments) {
        if (document->relativePath == relativePath && control == document->control)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::ExternalDocument*QmlCodeEditorWidget::getExternal(const QString& fullPath)
{
    for (ExternalDocument* document : m_externalDocuments) {
        if (document->fullPath == fullPath)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::GlobalDocument* QmlCodeEditorWidget::createGlobal(const QString& relativePath)
{
    GlobalDocument* document = new GlobalDocument;
    document->type = Global;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setPlainText(rdfile(globalDir() + separator() + relativePath));
    m_globalDocuments.append(document);
    return document;
}

QmlCodeEditorWidget::InternalDocument* QmlCodeEditorWidget::createInternal(Control* control, const QString& relativePath)
{
    InternalDocument* document = new InternalDocument;
    document->type = Internal;
    document->control = control;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setPlainText(rdfile(control->dir() + separator() + relativePath));
    m_internalDocuments.append(document);
    return document;
}

QmlCodeEditorWidget::ExternalDocument* QmlCodeEditorWidget::createExternal(const QString& fullPath)
{
    ExternalDocument* document = new ExternalDocument;
    document->type = External;
    document->fullPath = fullPath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setPlainText(rdfile(fullPath));
    m_externalDocuments.append(document);
    return document;
}

void QmlCodeEditorWidget::sweep()
{
    m_splitter->setStretchFactor(0, 30);
    m_splitter->setStretchFactor(1, 9);

    m_codeEditor->sweep();
    m_fileExplorer->sweep();

    // TODO: m_openDocument = new untitled external document
    m_fileExplorer->setRootPath("/Users/omergoktas/Desktop");
}

void QmlCodeEditorWidget::save()
{

}

void QmlCodeEditorWidget::close()
{

}

void QmlCodeEditorWidget::setFileExplorerVisible(bool visible)
{
    m_splitter->handle(1)->setDisabled(!visible);
    m_fileExplorer->setHidden(!visible);
}

bool QmlCodeEditorWidget::documentExists(QmlCodeEditorWidget::Document* document)
{
    if (document->type == Global)
        return m_globalDocuments.contains(static_cast<GlobalDocument*>(document));
    if (document->type == Internal)
        return m_internalDocuments.contains(static_cast<InternalDocument*>(document));
    if (document->type == External)
        return m_externalDocuments.contains(static_cast<ExternalDocument*>(document));
    Q_ASSERT(0);
    return false;
}

void QmlCodeEditorWidget::openDocument(Document* document)
{
    if (!documentExists(document))
        newDocument(document);

    m_openDocument = document;

    m_codeEditor->setDocument(m_openDocument->document);
    m_codeEditor->setTextCursor(m_openDocument->textCursor);

    if (m_openDocument->type == Global)
        m_fileExplorer->setRootPath(globalDir());
    else if (m_openDocument->type == Internal)
        m_fileExplorer->setRootPath(static_cast<InternalDocument*>(m_openDocument)->control->dir());
    else
        m_fileExplorer->setRootPath(dname(static_cast<ExternalDocument*>(m_openDocument)->fullPath));
}

void QmlCodeEditorWidget::newDocument(QmlCodeEditorWidget::Document* document)
{
//    switch (document->type) {
//    case Global: {

//    } break;
//    case Internal: {
//        InternalDocument* doc = new InternalDocument;
//        doc->type = Internal;
//        doc->control = control;
//        doc->document = new QmlCodeDocument(m_codeEditor);
//        doc->document->setPlainText(rdfile(control->url()));

//        m_internalDocuments.append(document);
//    } break;
//    case External: {

//    } break;
//    default:
//        Q_ASSERT(0);
//        break;
//    }
}


//QmlCodeEditorWidget::Document* QmlCodeEditorWidget::createAndAppendNewDocument(
//        const QString& path,
//        QmlCodeEditorWidget::DocumentType type) const
//{
//    Q_ASSERT(type != Internal);

//    ExternalDocument* document = new ExternalDocument;
//    document->type = type;
//    document->path = path;
//    document->document = new QmlCodeDocument(m_codeEditor);
//    document->document->setPlainText(rdfile(path));

//    if (type == Regular)
//        m_regularDocuments.append(document);
//    else
//        m_globalDocuments.append(document);

//    return document;
//}

//QmlCodeEditorWidget::Document* QmlCodeEditorWidget::createAndAppendNewDocument(Control* control) const
//{
//    Q_ASSERT(type == Internal);

//    InternalDocument* document = new InternalDocument;
//    document->type = Internal;
//    document->control = control;
//    document->document = new QmlCodeDocument(m_codeEditor);
//    document->document->setPlainText(rdfile(control->url()));

//    m_internalDocuments.append(document);

//    return document;
//}