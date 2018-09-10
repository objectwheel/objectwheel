#include <qmlcodeeditorwidget.h>
#include <filemanager.h>
#include <qmlcodeeditor.h>
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
#include <QComboBox>

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

// Change the relativePath parameter on public methods with filePath (like control.url())
// Fix different fonts of different QmlCodeDocuments
// Drag & drop files from desktop

#define global(x) static_cast<QmlCodeEditorWidget::GlobalDocument*>((x))
#define internal(x) static_cast<QmlCodeEditorWidget::InternalDocument*>((x))
#define external(x) static_cast<QmlCodeEditorWidget::ExternalDocument*>((x))
#define globalDir() SaveUtils::toGlobalDir(ProjectManager::dir())
#define internalDir(x) SaveUtils::toThisDir(internal((x))->control->dir())
#define externalDir(x) dname(external((x))->fullPath)
#define fullPath(x, y) (x) + separator() + (y)

enum ComboDataRole { DocumentRole = Qt::UserRole + 1, ControlRole };

namespace {

QmlCodeEditorWidget::GlobalDocument* lastGlobalDocument;
QmlCodeEditorWidget::InternalDocument* lastInternalDocument;
QmlCodeEditorWidget::ExternalDocument* lastExternalDocument;

void setupLastOpenedDocs(QmlCodeEditorWidget::Document* document)
{
    if (document->scope == QmlCodeEditorToolBar::Global)
        lastGlobalDocument = global(document);
    else if (document->scope == QmlCodeEditorToolBar::Internal)
        lastInternalDocument = internal(document);
    else
        lastExternalDocument = external(document);
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

QList<Control*> controls(const QList<QmlCodeEditorWidget::InternalDocument*>& documents)
{
    QSet<Control*> controlSet;
    for (QmlCodeEditorWidget::InternalDocument* document : documents)
        controlSet.insert(document->control);
    return controlSet.toList();
}
}

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

    connect(toolBar(), &QmlCodeEditorToolBar::saved,
            this, &QmlCodeEditorWidget::save);
    connect(toolBar(), &QmlCodeEditorToolBar::closed,
            this, &QmlCodeEditorWidget::close);
    connect(toolBar(), &QmlCodeEditorToolBar::showed,
            this, &QmlCodeEditorWidget::setFileExplorerVisible);
    connect(toolBar(), &QmlCodeEditorToolBar::pinned,
            this, &QmlCodeEditorWidget::pinned);
    connect(toolBar(), &QmlCodeEditorToolBar::scopeActivated,
            this, &QmlCodeEditorWidget::onScopeActivation);
    connect(toolBar(), &QmlCodeEditorToolBar::comboActivated,
            this, &QmlCodeEditorWidget::onComboActivation);

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

    m_openDocument = nullptr;
    lastGlobalDocument = nullptr;
    lastInternalDocument = nullptr;
    lastExternalDocument = nullptr;
    // TODO: m_openDocument = new untitled external document
}

void QmlCodeEditorWidget::setFileExplorerVisible(bool visible)
{
    m_splitter->handle(1)->setDisabled(!visible);
    m_fileExplorer->setHidden(!visible);
}

void QmlCodeEditorWidget::save()
{
    // TODO
}

void QmlCodeEditorWidget::close()
{
    // TODO
}

void QmlCodeEditorWidget::onScopeActivation(QmlCodeEditorToolBar::Scope scope)
{
    if (scope == QmlCodeEditorToolBar::Global)
        return openGlobal(QString());
    if (scope == QmlCodeEditorToolBar::Internal)
        return openInternal(nullptr, QString());
    if (scope == QmlCodeEditorToolBar::External)
        return openExternal(QString());
}

void QmlCodeEditorWidget::onComboActivation(QmlCodeEditorToolBar::Combo combo)
{
    Q_ASSERT(toolBar()->scope() != QmlCodeEditorToolBar::Global || combo != QmlCodeEditorToolBar::RightCombo);
    Q_ASSERT(toolBar()->scope() != QmlCodeEditorToolBar::External || combo != QmlCodeEditorToolBar::RightCombo);

    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Global)
        return openGlobal(leftCombo->currentText());
    if (toolBar()->scope() == QmlCodeEditorToolBar::External)
        return openExternal(fullPath(m_fileExplorer->rootPath(), leftCombo->currentText()));
    if (toolBar()->scope() == QmlCodeEditorToolBar::Internal)
        return openInternal(leftCombo->itemData(leftCombo->currentIndex(), ControlRole).value<Control*>(), rightCombo->currentText());
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

void QmlCodeEditorWidget::openGlobal(const QString& relativePath)
{
    if (relativePath.isEmpty())
        return openDocument(lastGlobalDocument);
    if (warnIfNotATextFile(fullPath(globalDir(), relativePath)))
        return;
    if (!globalExists(relativePath))
        return openDocument(addGlobal(relativePath));
    openDocument(getGlobal(relativePath));
}

void QmlCodeEditorWidget::openInternal(Control* control, const QString& relativePath)
{
    if (!control || relativePath.isEmpty())
        return openDocument(lastInternalDocument);
    if (warnIfNotATextFile(fullPath(SaveUtils::toThisDir(control->dir()), relativePath)))
        return;
    if (!internalExists(control, relativePath))
        return openDocument(addInternal(control, relativePath));
    openDocument(getInternal(control, relativePath));
}

void QmlCodeEditorWidget::openExternal(const QString& fullPath)
{
    if (fullPath.isEmpty())
        return openDocument(lastExternalDocument);
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
        const int i = leftCombo->count();
        leftCombo->addItem(document->relativePath);
        leftCombo->setItemData(i, document->relativePath, Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

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

    if (toolBar()->scope() == QmlCodeEditorToolBar::External) {
        QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
        QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);
        const int i = leftCombo->count();
        const int j = rightCombo->count();
        leftCombo->addItem(control->id());
        leftCombo->setItemData(i, control->id() + "::" + control->uid(), Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(control), ComboDataRole::DocumentRole);
        rightCombo->addItem(document->relativePath);
        rightCombo->setItemData(j, document->relativePath, Qt::ToolTipRole);
        rightCombo->setItemData(j, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

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
        const int i = leftCombo->count();
        leftCombo->addItem(fname(document->fullPath));
        leftCombo->setItemData(i, document->fullPath, Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

    return document;
}

void QmlCodeEditorWidget::openDocument(Document* document)
{
    if (!document)
        return;

    if (m_openDocument == document)
        return;

    setupToolBar(document);
    setupCodeEditor(document);
    setupFileExplorer(document);
    setupLastOpenedDocs(document);

    m_openDocument = document;

    emit opened();
}

void QmlCodeEditorWidget::setupCodeEditor(QmlCodeEditorWidget::Document* document)
{
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

void QmlCodeEditorWidget::setupToolBar(Document* document)
{
    QmlCodeEditorToolBar::Scope scope = document->scope;
    bool refresh = !m_openDocument || m_openDocument->scope != document->scope;
    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    if (refresh) {
        leftCombo->hide();
        leftCombo->clear();
        rightCombo->hide();
        rightCombo->clear();
        toolBar()->setScope(scope);

        switch (scope) {
        case QmlCodeEditorToolBar::Global:
            leftCombo->show();
            leftCombo->setToolTip(tr("Relative file path of the open document within the Global Resources"));
            for (GlobalDocument* doc : m_globalDocuments) {
                const int i = leftCombo->count();
                leftCombo->addItem(doc->relativePath);
                leftCombo->setItemData(i, doc->relativePath, Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    leftCombo->setCurrentIndex(i);
            } break;

        case QmlCodeEditorToolBar::Internal:
            leftCombo->show();
            rightCombo->show();
            leftCombo->setToolTip(tr("Control name"));
            rightCombo->setToolTip(tr("Relative file path of the open document within the control"));
            for (Control* control : controls(m_internalDocuments)) {
                const int i = leftCombo->count();
                leftCombo->addItem(control->id());
                leftCombo->setItemData(i, control->id() + "::" + control->uid(), Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(control), ComboDataRole::ControlRole);
                const InternalDocument* doc = static_cast<InternalDocument*>(document);
                if (doc->control == control)
                    leftCombo->setCurrentIndex(i);
            } for (InternalDocument* doc : m_internalDocuments) {
                const int i = rightCombo->count();
                rightCombo->addItem(doc->relativePath);
                rightCombo->setItemData(i, doc->relativePath, Qt::ToolTipRole);
                rightCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    rightCombo->setCurrentIndex(i);
            } break;

        case QmlCodeEditorToolBar::External:
            leftCombo->show();
            leftCombo->setToolTip(tr("File name of the open document"));
            for (ExternalDocument* doc : m_externalDocuments) {
                const int i = leftCombo->count();
                leftCombo->addItem(fname(doc->fullPath));
                leftCombo->setItemData(i, doc->fullPath, Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    leftCombo->setCurrentIndex(i);
            } break;
        }
    } else {
        switch (scope) {
        case QmlCodeEditorToolBar::Global:
        case QmlCodeEditorToolBar::External:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, DocumentRole).value<Document*>() == document) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } break;

        case QmlCodeEditorToolBar::Internal:
            for (int i = 0; i < leftCombo->count(); ++i) {
                const InternalDocument* doc = static_cast<InternalDocument*>(document);
                if (leftCombo->itemData(i, ControlRole).value<Control*>() == doc->control) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } for (int i = 0; i < rightCombo->count(); ++i) {
                if (rightCombo->itemData(i, DocumentRole).value<Document*>() == document) {
                    rightCombo->setCurrentIndex(i);
                    break;
                }
            } break;
        }
    }
}

QmlCodeEditorToolBar* QmlCodeEditorWidget::toolBar() const
{
    return m_codeEditor->toolBar();
}

QSize QmlCodeEditorWidget::sizeHint() const
{
    return QSize(680, 680);
}