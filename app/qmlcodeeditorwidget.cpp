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
#include <QMimeDatabase>
#include <QMessageBox>

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

#define global(x) static_cast<GlobalDocument*>((x))
#define internal(x) static_cast<InternalDocument*>((x))
#define external(x) static_cast<ExternalDocument*>((x))
#define globalDir() SaveUtils::toGlobalDir(ProjectManager::dir())
#define internalDir(x) SaveUtils::toThisDir(internal((x))->control->dir())
#define externalDir(x) dname(external((x))->fullPath)
#define relativePath(x, y) QDir((x)).relativeFilePath((y))

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget *parent) : QWidget(parent)
  , m_splitter(new QSplitter(this))
  , m_codeEditor(new QmlCodeEditor(this))
  , m_fileExplorer(new FileExplorer(0))
{
    auto layout = new QVBoxLayout(this);
    layout->addWidget(m_splitter);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_codeEditor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_fileExplorer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

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

    connect(m_fileExplorer, &FileExplorer::fileOpened,
            this, &QmlCodeEditorWidget::onFileExplorerFileOpen);
}

int QmlCodeEditorWidget::count() const
{
    return m_globalDocuments.size() + m_internalDocuments.size() + m_externalDocuments.size();
}

void QmlCodeEditorWidget::sweep()
{
    m_splitter->setStretchFactor(0, 30);
    m_splitter->setStretchFactor(1, 9);

    m_codeEditor->sweep();
    m_fileExplorer->sweep();

    // TODO: m_openDocument = new untitled external document
}

void QmlCodeEditorWidget::setFileExplorerVisible(bool visible)
{
    m_splitter->handle(1)->setDisabled(!visible);
    m_fileExplorer->setHidden(!visible);
}

void QmlCodeEditorWidget::onFileExplorerFileOpen(const QString& filePath)
{
    QMimeDatabase mimeDatabase;
    const QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    if (!mimeType.isValid() || !mimeType.inherits("text/plain")) {
        QMessageBox::warning(this, tr("Oops"), tr("Qml Code Editor cannot open non-text files."));
        return;
    }

    if (m_openDocument->type == Global)
        return openGlobal(relativePath(globalDir(), filePath));
    if (m_openDocument->type == Internal)
        return openInternal(internal(m_openDocument)->control, relativePath(internalDir(m_openDocument), filePath));
    if (m_openDocument->type == External)
        return openExternal(filePath);
}

void QmlCodeEditorWidget::openGlobal(const QString& relativePath)
{
    if (!globalExists(relativePath))
        return openDocument(createGlobal(relativePath));
    openDocument(getGlobal(relativePath));
}

void QmlCodeEditorWidget::openInternal(Control* control, const QString& relativePath)
{
    if (!internalExists(control, relativePath))
        return openDocument(createInternal(control, relativePath));
    openDocument(getInternal(control, relativePath));
}

void QmlCodeEditorWidget::openExternal(const QString& fullPath)
{
    if (!externalExists(fullPath))
        return openDocument(createExternal(fullPath));
    openDocument(getExternal(fullPath));
}

bool QmlCodeEditorWidget::globalExists(const QString& relativePath) const
{
    for (GlobalDocument* document : m_globalDocuments) {
        if (document->relativePath == relativePath)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::internalExists(Control* control, const QString& relativePath) const
{
    for (InternalDocument* document : m_internalDocuments) {
        if (document->relativePath == relativePath && control == document->control)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::externalExists(const QString& fullPath) const
{
    for (ExternalDocument* document : m_externalDocuments) {
        if (document->fullPath == fullPath)
            return true;
    }
    return false;
}

QmlCodeEditorWidget::GlobalDocument* QmlCodeEditorWidget::getGlobal(const QString& relativePath) const
{
    for (GlobalDocument* document : m_globalDocuments) {
        if (document->relativePath == relativePath)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::InternalDocument* QmlCodeEditorWidget::getInternal(Control* control,
                                                                        const QString& relativePath) const
{
    for (InternalDocument* document : m_internalDocuments) {
        if (document->relativePath == relativePath && control == document->control)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::ExternalDocument*QmlCodeEditorWidget::getExternal(const QString& fullPath) const
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
    document->document->setFilePath(globalDir() + separator() + relativePath);
    document->document->setPlainText(rdfile(globalDir() + separator() + relativePath));
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);
    m_globalDocuments.append(document);
    return document;
}

QmlCodeEditorWidget::InternalDocument* QmlCodeEditorWidget::createInternal(Control* control,
                                                                           const QString& relativePath)
{
    InternalDocument* document = new InternalDocument;
    document->type = Internal;
    document->control = control;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(SaveUtils::toThisDir(control->dir()) + separator() + relativePath);
    document->document->setPlainText(rdfile(SaveUtils::toThisDir(control->dir()) + separator() + relativePath));
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);
    m_internalDocuments.append(document);
    return document;
}

QmlCodeEditorWidget::ExternalDocument* QmlCodeEditorWidget::createExternal(const QString& fullPath)
{
    ExternalDocument* document = new ExternalDocument;
    document->type = External;
    document->fullPath = fullPath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(fullPath);
    document->document->setPlainText(rdfile(fullPath));
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);
    m_externalDocuments.append(document);
    return document;
}

void QmlCodeEditorWidget::save()
{

}

void QmlCodeEditorWidget::close()
{

}

void QmlCodeEditorWidget::openDocument(Document* document)
{
    if (m_openDocument == document)
        return;

    m_openDocument = document;

    m_codeEditor->setCodeDocument(m_openDocument->document);
    m_codeEditor->setTextCursor(m_openDocument->textCursor);

    if (m_openDocument->type == Global)
        m_fileExplorer->setRootPath(globalDir());
    else if (m_openDocument->type == Internal)
        m_fileExplorer->setRootPath(internalDir(m_openDocument));
    else
        m_fileExplorer->setRootPath(externalDir(m_openDocument));

    emit activated();
}

QSize QmlCodeEditorWidget::sizeHint() const
{
    return QSize(680, 680);
}