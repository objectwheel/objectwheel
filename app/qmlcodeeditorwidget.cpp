#include <qmlcodeeditorwidget.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
#include <fileexplorer.h>
#include <qmlcodedocument.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <control.h>
#include <utilityfunctions.h>

#include <QTimer>
#include <QApplication>
#include <QToolBar>
#include <QToolButton>
#include <QSplitter>
#include <QLayout>
#include <QMimeDatabase>
#include <QMimeData>
#include <QMessageBox>
#include <QComboBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QWindow>
#include <QScreen>

// FIXME:
// What happens if a global open file get renamed
// What happens if a global open file get deleted
// What happens if a global open file get overwritten/content changed outside

// What happens if a external open file get renamed
// What happens if a external open file get deleted
// What happens if a external open file get overwritten/content changed outside

// What happens if a internal open file get renamed
// What happens if a internal open file get deleted
// What happens if a internal open file get overwritten/content changed outside
// What happens if a control get deleted
// What happens if a control's dir changes
// What happens if a control's id changes (within code editor/out of code editor)
// What happens to the file explorer's root path if a control's dir changes

#define MARK_ASTERISK "*"
#define global(x) static_cast<QmlCodeEditorWidget::GlobalDocument*>((x))
#define internal(x) static_cast<QmlCodeEditorWidget::InternalDocument*>((x))
#define external(x) static_cast<QmlCodeEditorWidget::ExternalDocument*>((x))
#define globalDir() SaveUtils::toGlobalDir(ProjectManager::dir())
#define internalDir(x) SaveUtils::toThisDir(internal((x))->control->dir())
#define externalDir(x) dname(external((x))->fullPath)
#define fullPath(x, y) (x) + separator() + (y)
#define modified(x, y) (x)->isModified() ? ((y) + MARK_ASTERISK) : (y)
#define modifiedControlId(x) controlModified((x)) ? (x)->id() + MARK_ASTERISK : (x)->id()
extern const char* TOOL_KEY;

enum ComboDataRole { DocumentRole = Qt::UserRole + 1, ControlRole };

namespace {

bool g_fileExplorerHid = false;
QmlCodeEditorWidget::GlobalDocument* g_lastGlobalDocument;
QmlCodeEditorWidget::InternalDocument* g_lastInternalDocument;
QmlCodeEditorWidget::ExternalDocument* g_lastExternalDocument;

void setupLastOpenedDocs(QmlCodeEditorWidget::Document* document)
{
    if (document->scope == QmlCodeEditorToolBar::Global)
        g_lastGlobalDocument = global(document);
    else if (document->scope == QmlCodeEditorToolBar::Internal)
        g_lastInternalDocument = internal(document);
    else
        g_lastExternalDocument = external(document);
}

int warnIfModifiedContent(const QmlCodeEditorWidget::Document* document)
{
    const QmlCodeDocument* qmlDoc = document->document;
    if (qmlDoc->isModified()) {
        return QMessageBox::warning(
                    0,
                    QObject::tr("Unsaved Content"),
                    QObject::tr("The document contains unsaved content. "
                                "What would you like to do with the document?"),
                    QMessageBox::Discard | QMessageBox::Save | QMessageBox::Cancel, QMessageBox::Cancel);
    }
    return QMessageBox::Discard;
}

bool warnIfNotATextFile(const QString& filePath)
{
    QMimeDatabase mimeDatabase;
    const QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    if (!mimeType.isValid() || !mimeType.inherits("text/plain")) {
        return QMessageBox::warning(
                    0,
                    QObject::tr("Oops"),
                    QObject::tr("Qml Code Editor cannot open non-text files."));
    }
    return false;
}

bool warnIfFileWriteFails(const QString& filePath, const QString& content)
{
    if (!wrfile(filePath, content.toUtf8())) {
        return QMessageBox::critical(
                    0,
                    QObject::tr("Oops"),
                    QObject::tr("File write failed. File path: %1").arg(filePath));
    }
    return false;
}

QString choppedPath(const QString& path)
{
    QString choppedPath(path);
    if (choppedPath.right(1) == MARK_ASTERISK)
        choppedPath.chop(1);
    return choppedPath;
}

QList<Control*> controls(const QList<QmlCodeEditorWidget::InternalDocument*>& documents)
{
    QSet<Control*> controlSet;
    for (QmlCodeEditorWidget::InternalDocument* document : documents)
        controlSet.insert(document->control);
    return controlSet.toList();
}
}

QmlCodeEditorWidget::QmlCodeEditorWidget(QWidget* parent) : QWidget(parent)
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

    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->addWidget(m_codeEditor);
    m_splitter->addWidget(m_fileExplorer);
    m_splitter->setCollapsible(0, false);
    m_splitter->setCollapsible(1, false);
    m_splitter->setHandleWidth(0);

    setAcceptDrops(true);

    connect(toolBar(), &QmlCodeEditorToolBar::saved,
            this, &QmlCodeEditorWidget::save);
    connect(toolBar(), &QmlCodeEditorToolBar::closed,
            this, &QmlCodeEditorWidget::close);
    connect(toolBar(), &QmlCodeEditorToolBar::showed,
            this, &QmlCodeEditorWidget::setFileExplorerVisible);
    connect(toolBar(), &QmlCodeEditorToolBar::pinned,
            this, &QmlCodeEditorWidget::onPinActivation);
    connect(toolBar(), &QmlCodeEditorToolBar::scopeActivated,
            this, &QmlCodeEditorWidget::onScopeActivation);
    connect(toolBar(), &QmlCodeEditorToolBar::comboActivated,
            this, &QmlCodeEditorWidget::onComboActivation);
    connect(toolBar(), &QmlCodeEditorToolBar::newFile,
            this, &QmlCodeEditorWidget::onNewExternalFile);
    connect(toolBar(), &QmlCodeEditorToolBar::openFile,
            this, &QmlCodeEditorWidget::onOpenExternalFile);
    connect(m_fileExplorer, &FileExplorer::fileOpened,
            this, &QmlCodeEditorWidget::onFileExplorerFileOpen);
    connect(m_codeEditor, &QmlCodeEditor::modificationChanged,
            this, &QmlCodeEditorWidget::onModificationChange, Qt::QueuedConnection);
}

int QmlCodeEditorWidget::count() const
{
    return m_globalDocuments.size() + m_internalDocuments.size() + m_externalDocuments.size();
}

void QmlCodeEditorWidget::sweep()
{
    m_codeEditor->sweep();
    m_fileExplorer->sweep();

    m_splitter->setStretchFactor(0, 30);
    m_splitter->setStretchFactor(1, 9);

    m_openDocument = nullptr;
    g_fileExplorerHid = false;
    g_lastGlobalDocument = nullptr;
    g_lastInternalDocument = nullptr;
    g_lastExternalDocument = nullptr;

    setFileExplorerVisible(false);
    toolBar()->setHiddenActions(QmlCodeEditorToolBar::AllActions);

    for (GlobalDocument* document : m_globalDocuments) {
        delete document->document;
        delete document;
    }
    for (InternalDocument* document : m_internalDocuments) {
        delete document->document;
        delete document;
    }
    for (ExternalDocument* document : m_externalDocuments) {
        delete document->document;
        delete document;
    }
    m_globalDocuments.clear();
    m_internalDocuments.clear();
    m_externalDocuments.clear();
}

void QmlCodeEditorWidget::setFileExplorerVisible(bool visible)
{
    m_splitter->handle(1)->setDisabled(!visible);
    m_fileExplorer->setHidden(!visible);
}

void QmlCodeEditorWidget::onNewExternalFile()
{
    const QString& fullPath = QFileDialog::getSaveFileName(
                this,
                tr("New External File"),
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                tr("Text files (*.txt *.qml *.js *.json *.xml *.html *.htm *.css)"));

    if (fullPath.isEmpty())
        return;

    if (!mkfile(fullPath))
        return;

    openExternal(fullPath);
}

void QmlCodeEditorWidget::onOpenExternalFile()
{
    const QString& fullPath = QFileDialog::getOpenFileName(
                this,
                tr("New External File"),
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                tr("Text files (*.txt *.qml *.js *.json *.xml *.html *.htm *.css)"));

    if (fullPath.isEmpty())
        return;

    openExternal(fullPath);
}

void QmlCodeEditorWidget::onModificationChange()
{
    if (!m_openDocument)
        return;

    Q_ASSERT(m_openDocument->document == m_codeEditor->codeDocument());

    QmlCodeEditorToolBar::Scope scope = m_openDocument->scope;
    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    switch (scope) {
    case QmlCodeEditorToolBar::Global:
        for (int i = 0; i < leftCombo->count(); ++i) {
            GlobalDocument* doc = leftCombo->itemData(i, DocumentRole).value<GlobalDocument*>();
            if (doc == m_openDocument) {
                leftCombo->setItemText(i, modified(doc->document, doc->relativePath));
                break;
            }
        } break;

    case QmlCodeEditorToolBar::External:
        for (int i = 0; i < leftCombo->count(); ++i) {
            ExternalDocument* doc = leftCombo->itemData(i, DocumentRole).value<ExternalDocument*>();
            if (doc == m_openDocument) {
                leftCombo->setItemText(i, modified(doc->document, fname(doc->fullPath)));
                break;
            }
        } break;

    case QmlCodeEditorToolBar::Internal:
        for (int i = 0; i < leftCombo->count(); ++i) {
            Control* control = leftCombo->itemData(i, ControlRole).value<Control*>();
            if (control == internal(m_openDocument)->control) {
                leftCombo->setItemText(i, modifiedControlId(control));
                break;
            }
        } for (int i = 0; i < rightCombo->count(); ++i) {
            InternalDocument* doc = rightCombo->itemData(i, DocumentRole).value<InternalDocument*>();
            if (doc == m_openDocument) {
                rightCombo->setItemText(i, modified(doc->document, doc->relativePath));
                break;
            }
        } break;
    }
}

void QmlCodeEditorWidget::onPinActivation(bool pinned)
{
    const QRect& geo = geometry();
    setWindowFlags(pinned ? Qt::Widget : Qt::Tool);
    if (!pinned) {
        QTimer::singleShot(200, [=] {
            show();
            setGeometry(geo);
            UtilityFunctions::centralizeWidget(this);
        });
    } else {
        show();
    }
}

void QmlCodeEditorWidget::save()
{
    if (!m_openDocument)
        return;

    Q_ASSERT(m_openDocument->document->isModified());

    QString path;
    if (m_openDocument->scope == QmlCodeEditorToolBar::Global)
        path = fullPath(globalDir(), global(m_openDocument)->relativePath);
    else if (m_openDocument->scope == QmlCodeEditorToolBar::Internal)
        path = fullPath(internalDir(m_openDocument), internal(m_openDocument)->relativePath);
    else
        path = external(m_openDocument)->fullPath;

    Q_ASSERT(!path.isEmpty());

    for (SaveFilter* saveFilter : m_saveFilters)
        saveFilter->beforeSave(m_openDocument);

    if (warnIfFileWriteFails(path, m_openDocument->document->toPlainText()))
        return;

    m_openDocument->document->setModified(false);

    for (SaveFilter* saveFilter : m_saveFilters)
        saveFilter->afterSave(m_openDocument);
}

void QmlCodeEditorWidget::close()
{
    if (!m_openDocument)
        return;

    switch (warnIfModifiedContent(m_openDocument)) {
    case QMessageBox::Save:
        save();
        break;
    case QMessageBox::Discard:
        break;
    case QMessageBox::Cancel:
        return;
    default:
        Q_ASSERT(0);
        return;
    }

    int indexForRemoval = -1;
    Document* nextDocument = nullptr;
    QmlCodeEditorToolBar::Scope scope = m_openDocument->scope;
    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    if (scope == QmlCodeEditorToolBar::Global) {
        g_lastGlobalDocument = nullptr;
        m_globalDocuments.removeOne(global(m_openDocument));
        for (int i = 0; i < leftCombo->count(); ++i) {
            if (leftCombo->itemData(i, DocumentRole).value<GlobalDocument*>() == m_openDocument) {
                indexForRemoval = i;
                break;
            }
        }
        if (indexForRemoval >= 0)
            leftCombo->removeItem(indexForRemoval);
        if (leftCombo->count() > 0)
            nextDocument = leftCombo->itemData(0, DocumentRole).value<GlobalDocument*>();
        toolBar()->setScopeWide(QmlCodeEditorToolBar::Global, !m_globalDocuments.isEmpty());
    } else if (scope == QmlCodeEditorToolBar::Internal) {
        g_lastInternalDocument = nullptr;
        m_internalDocuments.removeOne(internal(m_openDocument));
        for (int i = 0; i < rightCombo->count(); ++i) {
            if (rightCombo->itemData(i, DocumentRole).value<InternalDocument*>() == m_openDocument) {
                indexForRemoval = i;
                break;
            }
        }
        if (indexForRemoval >= 0)
            rightCombo->removeItem(indexForRemoval);
        if (rightCombo->count() > 0) {
            nextDocument = rightCombo->itemData(0, DocumentRole).value<InternalDocument*>();
        } else {
            indexForRemoval = -1;
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, ControlRole).value<Control*>() == internal(m_openDocument)->control) {
                    indexForRemoval = i;
                    break;
                }
            }
            if (indexForRemoval >= 0)
                leftCombo->removeItem(indexForRemoval);
            if (leftCombo->count() > 0) {
                Q_ASSERT(m_internalDocuments.size() > 0);
                nextDocument = m_internalDocuments.last();
            }
        }
        toolBar()->setScopeWide(QmlCodeEditorToolBar::Internal, !m_internalDocuments.isEmpty());
    } else {
        g_lastExternalDocument = nullptr;
        m_externalDocuments.removeOne(external(m_openDocument));
        for (int i = 0; i < leftCombo->count(); ++i) {
            if (leftCombo->itemData(i, DocumentRole).value<ExternalDocument*>() == m_openDocument) {
                indexForRemoval = i;
                break;
            }
        }
        if (indexForRemoval >= 0)
            leftCombo->removeItem(indexForRemoval);
        if (leftCombo->count() > 0)
            nextDocument = leftCombo->itemData(0, DocumentRole).value<ExternalDocument*>();
        toolBar()->setScopeWide(QmlCodeEditorToolBar::External, !m_externalDocuments.isEmpty());
    }

    Q_ASSERT(m_openDocument != nextDocument);

    Document* deletion = m_openDocument;
    showNoDocumentsOpen();

    delete deletion->document;
    delete deletion;

    if (nextDocument)
        openDocument(nextDocument);
}

void QmlCodeEditorWidget::showNoDocumentsOpen()
{
    QmlCodeEditorToolBar::DocumentActions hiddenActions = QmlCodeEditorToolBar::AllActions;
    if (toolBar()->scope() == QmlCodeEditorToolBar::External)
        hiddenActions &= ~QmlCodeEditorToolBar::FileActions;
    m_openDocument = nullptr;
    m_codeEditor->setNoDocsVisible(true);
    toolBar()->setHiddenActions(hiddenActions);
    if (m_fileExplorer->isVisible()) {
        g_fileExplorerHid = true;
        setFileExplorerVisible(false);
    }
}

void QmlCodeEditorWidget::onScopeActivation(QmlCodeEditorToolBar::Scope scope)
{
    if (scope == QmlCodeEditorToolBar::Global && g_lastGlobalDocument)
        return openGlobal(QString());
    if (scope == QmlCodeEditorToolBar::Internal && g_lastInternalDocument)
        return openInternal(nullptr, QString());
    if (scope == QmlCodeEditorToolBar::External && g_lastExternalDocument)
        return openExternal(QString());
    showNoDocumentsOpen();
}

void QmlCodeEditorWidget::onComboActivation(QmlCodeEditorToolBar::Combo combo)
{
    Q_ASSERT(toolBar()->scope() != QmlCodeEditorToolBar::Global || combo != QmlCodeEditorToolBar::RightCombo);
    Q_ASSERT(toolBar()->scope() != QmlCodeEditorToolBar::External || combo != QmlCodeEditorToolBar::RightCombo);

    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Global)
        return openGlobal(choppedPath(leftCombo->currentText()));
    if (toolBar()->scope() == QmlCodeEditorToolBar::External)
        return openExternal(fullPath(m_fileExplorer->rootPath(), choppedPath(leftCombo->currentText())));
    if (toolBar()->scope() == QmlCodeEditorToolBar::Internal) {
        Control* control = leftCombo->itemData(leftCombo->currentIndex(), ControlRole).value<Control*>();
        if (combo == QmlCodeEditorToolBar::RightCombo)
            return openInternal(control, choppedPath(rightCombo->currentText()));
        if (combo == QmlCodeEditorToolBar::LeftCombo) {
            InternalDocument* lastDoc = control->property("ow_last_document").value<InternalDocument*>();
            return openDocument(lastDoc);
        }
    }
}

void QmlCodeEditorWidget::onFileExplorerFileOpen(const QString& relativePath)
{
    if (m_openDocument->scope == QmlCodeEditorToolBar::Global)
        return openGlobal(relativePath);
    if (m_openDocument->scope == QmlCodeEditorToolBar::Internal)
        return openInternal(internal(m_openDocument)->control, relativePath);
    if (m_openDocument->scope == QmlCodeEditorToolBar::External)
        return openExternal(fullPath(m_fileExplorer->rootPath(), relativePath));
}

bool QmlCodeEditorWidget::documentExists(QmlCodeEditorWidget::Document* document) const
{
    for (GlobalDocument* doc : m_globalDocuments) {
        if (doc == document)
            return true;
    }
    for (InternalDocument* doc : m_internalDocuments) {
        if (doc == document)
            return true;
    }
    for (ExternalDocument* doc : m_externalDocuments) {
        if (doc == document)
            return true;
    }
    return false;
}

void QmlCodeEditorWidget::openGlobal(const QString& relativePath)
{
    if (relativePath.isEmpty())
        return openDocument(g_lastGlobalDocument);
    if (!exists(fullPath(globalDir(), relativePath)))
        return (void) (qWarning() << tr("openGlobal: File not exists."));
    if (warnIfNotATextFile(fullPath(globalDir(), relativePath)))
        return;
    if (!globalExists(relativePath))
        return openDocument(addGlobal(relativePath));
    openDocument(getGlobal(relativePath));
}

void QmlCodeEditorWidget::openInternal(Control* control, const QString& relativePath)
{
    if (!control || relativePath.isEmpty())
        return openDocument(g_lastInternalDocument);
    if (!exists(fullPath(SaveUtils::toThisDir(control->dir()), relativePath)))
        return (void) (qWarning() << tr("openInternal: File not exists."));
    if (warnIfNotATextFile(fullPath(SaveUtils::toThisDir(control->dir()), relativePath)))
        return;
    if (!internalExists(control, relativePath))
        return openDocument(addInternal(control, relativePath));
    openDocument(getInternal(control, relativePath));
}

void QmlCodeEditorWidget::openExternal(const QString& fullPath)
{
    if (fullPath.isEmpty())
        return openDocument(g_lastExternalDocument);
    if (!exists(fullPath))
        return (void) (qWarning() << tr("openExternal: File not exists."));
    if (warnIfNotATextFile(fullPath))
        return;
    if (!externalExists(fullPath))
        return openDocument(addExternal(fullPath));
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

QmlCodeEditorWidget::GlobalDocument* QmlCodeEditorWidget::addGlobal(const QString& relativePath)
{
    GlobalDocument* document = new GlobalDocument;
    document->scope = QmlCodeEditorToolBar::Global;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(globalDir() + separator() + relativePath);
    document->document->setPlainText(rdfile(globalDir() + separator() + relativePath));
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);

    m_globalDocuments.append(document);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Global) {
        QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
        int i = leftCombo->count();
        leftCombo->addItem(document->relativePath);
        leftCombo->setItemData(i, document->relativePath, Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

    toolBar()->setScopeWide(QmlCodeEditorToolBar::Global, true);

    return document;
}

QmlCodeEditorWidget::InternalDocument* QmlCodeEditorWidget::addInternal(Control* control,
                                                                        const QString& relativePath)
{
    InternalDocument* document = new InternalDocument;
    document->scope = QmlCodeEditorToolBar::Internal;
    document->control = control;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(fullPath(SaveUtils::toThisDir(control->dir()), relativePath));
    document->document->setPlainText(rdfile(fullPath(SaveUtils::toThisDir(control->dir()), relativePath)));
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);

    m_internalDocuments.append(document);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Internal) {
        if (!controlExists(control)) {
            QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
            int i = leftCombo->count();
            leftCombo->addItem(control->id());
            leftCombo->setItemData(i, control->id() + "::" + control->uid(), Qt::ToolTipRole);
            leftCombo->setItemData(i, QVariant::fromValue(control), ComboDataRole::DocumentRole);
        }
        QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);
        int i = rightCombo->count();
        rightCombo->addItem(document->relativePath);
        rightCombo->setItemData(i, document->relativePath, Qt::ToolTipRole);
        rightCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

    toolBar()->setScopeWide(QmlCodeEditorToolBar::Internal, true);

    return document;
}

QmlCodeEditorWidget::ExternalDocument* QmlCodeEditorWidget::addExternal(const QString& fullPath)
{
    ExternalDocument* document = new ExternalDocument;
    document->scope = QmlCodeEditorToolBar::External;
    document->fullPath = fullPath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(fullPath);
    document->document->setPlainText(rdfile(fullPath));
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);

    m_externalDocuments.append(document);

    if (toolBar()->scope() == QmlCodeEditorToolBar::External) {
        QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
        int i = leftCombo->count();
        leftCombo->addItem(fname(document->fullPath));
        leftCombo->setItemData(i, document->fullPath, Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

    toolBar()->setScopeWide(QmlCodeEditorToolBar::External, true);

    return document;
}

void QmlCodeEditorWidget::openDocument(Document* document)
{
    if (!document)
        return;

    m_codeEditor->setFocus();

    if (m_openDocument == document)
        return opened();

    if (!documentExists(document))
        return;

    if (m_openDocument)
        m_openDocument->textCursor = m_codeEditor->textCursor();

    setupToolBar(document);
    setupCodeEditor(document);
    setupFileExplorer(document);
    setupLastOpenedDocs(document);

    m_openDocument = document;

    emit opened();
}

void QmlCodeEditorWidget::setupCodeEditor(QmlCodeEditorWidget::Document* document)
{
    if (g_fileExplorerHid) {
        g_fileExplorerHid = false;
        setFileExplorerVisible(true);
    }
    m_codeEditor->setNoDocsVisible(false);
    m_codeEditor->setCodeDocument(document->document);
    m_codeEditor->setTextCursor(document->textCursor);
}

void QmlCodeEditorWidget::setupFileExplorer(QmlCodeEditorWidget::Document* document)
{
    if (document->scope == QmlCodeEditorToolBar::Global)
        return m_fileExplorer->setRootPath(globalDir());
    if (document->scope == QmlCodeEditorToolBar::Internal)
        return m_fileExplorer->setRootPath(internalDir(document));
    if (document->scope == QmlCodeEditorToolBar::External)
        return m_fileExplorer->setRootPath(externalDir(document));
}

void QmlCodeEditorWidget::dragEnterEvent(QDragEnterEvent* e)
{
    e->accept();
}

void QmlCodeEditorWidget::dragMoveEvent(QDragMoveEvent* e)
{
    e->accept();
}

void QmlCodeEditorWidget::dragLeaveEvent(QDragLeaveEvent* e)
{
    e->accept();
}

void QmlCodeEditorWidget::dropEvent(QDropEvent* e)
{
    const QMimeData* mimeData = e->mimeData();
    if (mimeData->hasUrls() && !(mimeData->hasText() && mimeData->text() == TOOL_KEY)) {
        e->accept();
        for (const QUrl& url : mimeData->urls()) {
            if (url.isLocalFile())
                openExternal(url.toLocalFile());
        }
    } else {
        e->ignore();
    }
}

void QmlCodeEditorWidget::closeEvent(QCloseEvent* e)
{
    QWidget::closeEvent(e);
    if (windowFlags() & Qt::Window) {
        QTimer::singleShot(200, this, [=] {
            toolBar()->setPinned(true);
            setWindowFlags(Qt::Widget);
            show();
        });
    }
}

void QmlCodeEditorWidget::setupToolBar(Document* document)
{
    QmlCodeEditorToolBar::Scope scope = document->scope;
    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    bool refresh = !m_openDocument
            || m_openDocument->scope != document->scope
            || (m_openDocument->scope == QmlCodeEditorToolBar::Internal
                && internal(m_openDocument)->control != internal(document)->control);

    if (refresh) {
        leftCombo->clear();
        rightCombo->clear();
        toolBar()->setScope(scope);

        switch (scope) {
        case QmlCodeEditorToolBar::Global:
            toolBar()->setHiddenActions(QmlCodeEditorToolBar::RightAction | QmlCodeEditorToolBar::FileActions);
            leftCombo->setToolTip(tr("Relative file path of the open document within the Global Resources"));
            for (GlobalDocument* doc : m_globalDocuments) {
                int i = leftCombo->count();
                leftCombo->addItem(modified(doc->document, doc->relativePath));
                leftCombo->setItemData(i, doc->relativePath, Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    leftCombo->setCurrentIndex(i);
            } break;

        case QmlCodeEditorToolBar::Internal:
            toolBar()->setHiddenActions(QmlCodeEditorToolBar::FileActions);
            leftCombo->setToolTip(tr("Control name"));
            rightCombo->setToolTip(tr("Relative file path of the open document within the control"));
            for (Control* control : controls(m_internalDocuments)) {
                int i = leftCombo->count();
                leftCombo->addItem(modifiedControlId(control));
                leftCombo->setItemData(i, control->id() + "::" + control->uid(), Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(control), ComboDataRole::ControlRole);
                if (internal(document)->control == control)
                    leftCombo->setCurrentIndex(i);
            } for (InternalDocument* doc : m_internalDocuments) {
                if (internal(document)->control == doc->control) {
                    int i = rightCombo->count();
                    rightCombo->addItem(modified(doc->document, doc->relativePath));
                    rightCombo->setItemData(i, doc->relativePath, Qt::ToolTipRole);
                    rightCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                    if (doc == document) {
                        rightCombo->setCurrentIndex(i);
                        internal(document)->control->setProperty("ow_last_document", QVariant::fromValue(doc));
                    }
                }
            } break;

        case QmlCodeEditorToolBar::External:
            toolBar()->setHiddenActions(QmlCodeEditorToolBar::RightAction);
            leftCombo->setToolTip(tr("File name of the open document"));
            for (ExternalDocument* doc : m_externalDocuments) {
                int i = leftCombo->count();
                leftCombo->addItem(modified(doc->document, fname(doc->fullPath)));
                leftCombo->setItemData(i, doc->fullPath, Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    leftCombo->setCurrentIndex(i);
            } break;
        }
    } else {
        switch (scope) {
        case QmlCodeEditorToolBar::Global:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, DocumentRole).value<GlobalDocument*>() == document) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } break;

        case QmlCodeEditorToolBar::External:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, DocumentRole).value<ExternalDocument*>() == document) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } break;

        case QmlCodeEditorToolBar::Internal:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, ControlRole).value<Control*>() == internal(document)->control) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } for (int i = 0; i < rightCombo->count(); ++i) {
                InternalDocument* doc = rightCombo->itemData(i, DocumentRole).value<InternalDocument*>();
                if (doc == document) {
                    rightCombo->setCurrentIndex(i);
                    internal(document)->control->setProperty("ow_last_document", QVariant::fromValue(doc));
                    break;
                }
            } break;
        }
    }
}

bool QmlCodeEditorWidget::controlExists(const Control* control)
{
    for (QmlCodeEditorWidget::InternalDocument* document : m_internalDocuments) {
        if (document->control == control)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::controlModified(const Control* control)
{
    Q_ASSERT(controlExists(control));
    for (QmlCodeEditorWidget::InternalDocument* document : m_internalDocuments) {
        if (document->control == control) {
            if (document->document->isModified())
                return true;
        }
    }
    return false;
}

QmlCodeEditor* QmlCodeEditorWidget::codeEditor() const
{
    return m_codeEditor;
}

QmlCodeEditorToolBar* QmlCodeEditorWidget::toolBar() const
{
    return m_codeEditor->toolBar();
}

QSize QmlCodeEditorWidget::sizeHint() const
{
    return QSize(680, 680);
}
