#include <qmlcodeeditorwidget.h>
#include <qmlcodeeditor.h>
#include <fileexplorer.h>
#include <qmlcodedocument.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <control.h>
#include <utilityfunctions.h>
#include <filesystemutils.h>

#include <QTimer>
#include <QApplication>
#include <QToolBar>
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
// What happens if a assets open file get renamed
// What happens if a assets open file get deleted
// What happens if a assets open file get overwritten/content changed outside

// What happens if a others open file get renamed
// What happens if a others open file get deleted
// What happens if a others open file get overwritten/content changed outside

// What happens if a designs open file get renamed
// What happens if a designs open file get deleted
// What happens if a designs open file get overwritten/content changed outside
// What happens if a control get deleted
// What happens if a control's dir changes
// What happens if a control's id changes (within code editor/out of code editor)
// What happens to the file explorer's root path if a control's dir changes

#define MARK_ASTERISK "*"
#define assets(x) static_cast<QmlCodeEditorWidget::AssetsDocument*>((x))
#define designs(x) static_cast<QmlCodeEditorWidget::DesignsDocument*>((x))
#define others(x) static_cast<QmlCodeEditorWidget::OthersDocument*>((x))
#define assetsDir() SaveUtils::toProjectAssetsDir(ProjectManager::dir())
#define designsDir(x) SaveUtils::toControlThisDir(designs((x))->control->dir())
#define othersDir(x) QFileInfo(others((x))->fullPath).path()
#define fullPath(x, y) (x) + '/' + (y)
#define modified(x, y) (x)->isModified() ? ((y) + MARK_ASTERISK) : (y)
#define modifiedControlId(x) controlModified((x)) ? (x)->id() + MARK_ASTERISK : (x)->id()

enum ComboDataRole { DocumentRole = Qt::UserRole + 1, ControlRole };

namespace {

bool g_fileExplorerHid = false;
QmlCodeEditorWidget::AssetsDocument* g_lastAssetsDocument;
QmlCodeEditorWidget::DesignsDocument* g_lastDesignsDocument;
QmlCodeEditorWidget::OthersDocument* g_lastOthersDocument;

void setupLastOpenedDocs(QmlCodeEditorWidget::Document* document)
{
    if (document->scope == QmlCodeEditorToolBar::Assets)
        g_lastAssetsDocument = assets(document);
    else if (document->scope == QmlCodeEditorToolBar::Designs)
        g_lastDesignsDocument = designs(document);
    else
        g_lastOthersDocument = others(document);
}

int warnIfModifiedContent(const QmlCodeEditorWidget::Document* document)
{
    const QmlCodeDocument* qmlDoc = document->document;
    if (qmlDoc->isModified()) {
        return UtilityFunctions::showMessage(nullptr,
                                             QObject::tr("Unsaved content"),
                                             QObject::tr("The document contains unsaved content. "
                                                         "What would you like to do with the document?"),
                                             QMessageBox::Warning,
                                             QMessageBox::Discard | QMessageBox::Save | QMessageBox::Cancel,
                                             QMessageBox::Cancel);
    }
    return QMessageBox::Discard;
}

bool warnIfNotATextFile(const QString& filePath)
{
    QMimeDatabase mimeDatabase;
    const QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    if (!mimeType.isValid() || !mimeType.inherits("text/plain")) {
        return UtilityFunctions::showMessage(
                    nullptr, QObject::tr("Oops"),
                    QObject::tr("Qml Code Editor cannot display binary content."));
    }
    return false;
}

bool warnIfFileWriteFails(const QString& filePath, const QString& content)
{
    QFile file(filePath);
    if (!file.open(QFile::WriteOnly)) {
        return UtilityFunctions::showMessage(
                    nullptr, QObject::tr("Oops"),
                    QObject::tr("Cannot open file. File path: %1").arg(filePath), QMessageBox::Critical);
    }
    if (file.write(content.toUtf8()) < 0) {
        return UtilityFunctions::showMessage(
                    nullptr, QObject::tr("Oops"),
                    QObject::tr("File write failed. File path: %1").arg(filePath), QMessageBox::Critical);
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

QList<Control*> controls(const QList<QmlCodeEditorWidget::DesignsDocument*>& documents)
{
    QSet<Control*> controlSet;
    for (QmlCodeEditorWidget::DesignsDocument* document : documents)
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

    m_splitter->addWidget(m_codeEditor);
    m_splitter->addWidget(m_fileExplorer);
    m_splitter->setFrameShape(QFrame::NoFrame);
    m_splitter->setChildrenCollapsible(false);
    m_splitter->setHandleWidth(0);

    setAcceptDrops(true);

    connect(toolBar(), &QmlCodeEditorToolBar::saved,
            this, &QmlCodeEditorWidget::saveOpen);
    connect(toolBar(), &QmlCodeEditorToolBar::savedAll,
            this, &QmlCodeEditorWidget::saveAll);
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
            this, &QmlCodeEditorWidget::onNewOthersFile);
    connect(toolBar(), &QmlCodeEditorToolBar::addFile,
            this, &QmlCodeEditorWidget::onAddOthersFile);
    connect(m_fileExplorer, &FileExplorer::fileOpened,
            this, &QmlCodeEditorWidget::onFileExplorerFileOpen);
    connect(m_codeEditor, &QmlCodeEditor::modificationChanged,
            this, [=] { onModificationChange(m_openDocument); }, Qt::QueuedConnection);
}

int QmlCodeEditorWidget::count() const
{
    return m_assetsDocuments.size() + m_designsDocuments.size() + m_OthersDocuments.size();
}

void QmlCodeEditorWidget::discharge()
{
    m_codeEditor->discharge();
    m_fileExplorer->discharge();

    m_splitter->setStretchFactor(0, 30);
    m_splitter->setStretchFactor(1, 9);

    m_openDocument = nullptr;
    g_fileExplorerHid = false;
    g_lastAssetsDocument = nullptr;
    g_lastDesignsDocument = nullptr;
    g_lastOthersDocument = nullptr;

    setFileExplorerVisible(false);
    toolBar()->setHiddenActions(QmlCodeEditorToolBar::AllActions);

    for (AssetsDocument* document : m_assetsDocuments) {
        delete document->document;
        delete document;
    }
    for (DesignsDocument* document : m_designsDocuments) {
        delete document->document;
        delete document;
    }
    for (OthersDocument* document : m_OthersDocuments) {
        delete document->document;
        delete document;
    }
    m_assetsDocuments.clear();
    m_designsDocuments.clear();
    m_OthersDocuments.clear();
}

void QmlCodeEditorWidget::setFileExplorerVisible(bool visible)
{
    m_splitter->handle(1)->setDisabled(!visible);
    m_fileExplorer->setHidden(!visible);
}

void QmlCodeEditorWidget::onNewOthersFile()
{
    const QString& fullPath = QFileDialog::getSaveFileName(
                this,
                tr("New External File"),
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                tr("Text files (*.txt *.qml *.js *.json *.xml *.html *.htm *.css)"));

    if (fullPath.isEmpty())
        return;

    if (!FileSystemUtils::makeFile(fullPath))
        return;

    openOthers(fullPath);
}

void QmlCodeEditorWidget::onAddOthersFile()
{
    const QString& fullPath = QFileDialog::getOpenFileName(
                this,
                tr("New External File"),
                QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
                tr("Text files (*.txt *.qml *.js *.json *.xml *.html *.htm *.css)"));

    if (fullPath.isEmpty())
        return;

    openOthers(fullPath);
}

void QmlCodeEditorWidget::onModificationChange(Document* document)
{
    if (!document)
        return;

    QmlCodeEditorToolBar::Scope scope = document->scope;
    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    switch (scope) {
    case QmlCodeEditorToolBar::Assets:
        for (int i = 0; i < leftCombo->count(); ++i) {
            AssetsDocument* doc = leftCombo->itemData(i, DocumentRole).value<AssetsDocument*>();
            if (doc == document) {
                leftCombo->setItemText(i, modified(doc->document, doc->relativePath));
                break;
            }
        } break;

    case QmlCodeEditorToolBar::Others:
        for (int i = 0; i < leftCombo->count(); ++i) {
            OthersDocument* doc = leftCombo->itemData(i, DocumentRole).value<OthersDocument*>();
            if (doc == document) {
                leftCombo->setItemText(i, modified(doc->document, QFileInfo(doc->fullPath).fileName()));
                break;
            }
        } break;

    case QmlCodeEditorToolBar::Designs:
        for (int i = 0; i < leftCombo->count(); ++i) {
            Control* control = leftCombo->itemData(i, ControlRole).value<Control*>();
            if (control == designs(document)->control) {
                leftCombo->setItemText(i, modifiedControlId(control));
                break;
            }
        } for (int i = 0; i < rightCombo->count(); ++i) {
            DesignsDocument* doc = rightCombo->itemData(i, DocumentRole).value<DesignsDocument*>();
            if (doc == document) {
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

void QmlCodeEditorWidget::saveOpen()
{
    if (!m_openDocument)
        return;
    Q_ASSERT(m_openDocument->document->isModified());
    save(m_openDocument);
}

void QmlCodeEditorWidget::saveAll()
{
    for (AssetsDocument* document : m_assetsDocuments)
        save(document);
    for (DesignsDocument* document : m_designsDocuments)
        save(document);
    for (OthersDocument* document : m_OthersDocuments)
        save(document);
}

void QmlCodeEditorWidget::save(QmlCodeEditorWidget::Document* document)
{
    if (!document->document->isModified())
        return;

    QString path;
    if (document->scope == QmlCodeEditorToolBar::Assets)
        path = fullPath(assetsDir(), assets(document)->relativePath);
    else if (document->scope == QmlCodeEditorToolBar::Designs)
        path = fullPath(designsDir(document), designs(document)->relativePath);
    else
        path = others(document)->fullPath;

    Q_ASSERT(!path.isEmpty());

    for (SaveFilter* saveFilter : m_saveFilters)
        saveFilter->beforeSave(document);

    if (warnIfFileWriteFails(path, document->document->toPlainText()))
        return;

    document->document->setModified(false);
    if (document != m_openDocument)
        onModificationChange(document);

    for (SaveFilter* saveFilter : m_saveFilters)
        saveFilter->afterSave(document);
}

void QmlCodeEditorWidget::close()
{
    if (!m_openDocument)
        return;

    switch (warnIfModifiedContent(m_openDocument)) {
    case QMessageBox::Save:
        saveOpen();
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

    if (scope == QmlCodeEditorToolBar::Assets) {
        g_lastAssetsDocument = nullptr;
        m_assetsDocuments.removeOne(assets(m_openDocument));
        for (int i = 0; i < leftCombo->count(); ++i) {
            if (leftCombo->itemData(i, DocumentRole).value<AssetsDocument*>() == m_openDocument) {
                indexForRemoval = i;
                break;
            }
        }
        if (indexForRemoval >= 0)
            leftCombo->removeItem(indexForRemoval);
        if (leftCombo->count() > 0)
            nextDocument = leftCombo->itemData(0, DocumentRole).value<AssetsDocument*>();
        toolBar()->setScopeWide(QmlCodeEditorToolBar::Assets, !m_assetsDocuments.isEmpty());
    } else if (scope == QmlCodeEditorToolBar::Designs) {
        g_lastDesignsDocument = nullptr;
        m_designsDocuments.removeOne(designs(m_openDocument));
        for (int i = 0; i < rightCombo->count(); ++i) {
            if (rightCombo->itemData(i, DocumentRole).value<DesignsDocument*>() == m_openDocument) {
                indexForRemoval = i;
                break;
            }
        }
        if (indexForRemoval >= 0)
            rightCombo->removeItem(indexForRemoval);
        if (rightCombo->count() > 0) {
            nextDocument = rightCombo->itemData(0, DocumentRole).value<DesignsDocument*>();
        } else {
            indexForRemoval = -1;
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, ControlRole).value<Control*>() == designs(m_openDocument)->control) {
                    indexForRemoval = i;
                    break;
                }
            }
            if (indexForRemoval >= 0)
                leftCombo->removeItem(indexForRemoval);
            if (leftCombo->count() > 0) {
                Q_ASSERT(m_designsDocuments.size() > 0);
                nextDocument = m_designsDocuments.last();
            }
        }
        toolBar()->setScopeWide(QmlCodeEditorToolBar::Designs, !m_designsDocuments.isEmpty());
    } else {
        g_lastOthersDocument = nullptr;
        m_OthersDocuments.removeOne(others(m_openDocument));
        for (int i = 0; i < leftCombo->count(); ++i) {
            if (leftCombo->itemData(i, DocumentRole).value<OthersDocument*>() == m_openDocument) {
                indexForRemoval = i;
                break;
            }
        }
        if (indexForRemoval >= 0)
            leftCombo->removeItem(indexForRemoval);
        if (leftCombo->count() > 0)
            nextDocument = leftCombo->itemData(0, DocumentRole).value<OthersDocument*>();
        toolBar()->setScopeWide(QmlCodeEditorToolBar::Others, !m_OthersDocuments.isEmpty());
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
    if (toolBar()->scope() == QmlCodeEditorToolBar::Others)
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
    if (scope == QmlCodeEditorToolBar::Assets && g_lastAssetsDocument)
        return openAssets(QString());
    if (scope == QmlCodeEditorToolBar::Designs && g_lastDesignsDocument)
        return openDesigns(nullptr, QString());
    if (scope == QmlCodeEditorToolBar::Others && g_lastOthersDocument)
        return openOthers(QString());
    showNoDocumentsOpen();
}

void QmlCodeEditorWidget::onComboActivation(QmlCodeEditorToolBar::Combo combo)
{
    Q_ASSERT(toolBar()->scope() != QmlCodeEditorToolBar::Assets || combo != QmlCodeEditorToolBar::RightCombo);
    Q_ASSERT(toolBar()->scope() != QmlCodeEditorToolBar::Others || combo != QmlCodeEditorToolBar::RightCombo);

    QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
    QComboBox* rightCombo = toolBar()->combo(QmlCodeEditorToolBar::RightCombo);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Assets)
        return openAssets(choppedPath(leftCombo->currentText()));
    if (toolBar()->scope() == QmlCodeEditorToolBar::Others)
        return openOthers(fullPath(m_fileExplorer->rootPath(), choppedPath(leftCombo->currentText())));
    if (toolBar()->scope() == QmlCodeEditorToolBar::Designs) {
        Control* control = leftCombo->itemData(leftCombo->currentIndex(), ControlRole).value<Control*>();
        if (combo == QmlCodeEditorToolBar::RightCombo)
            return openDesigns(control, choppedPath(rightCombo->currentText()));
        if (combo == QmlCodeEditorToolBar::LeftCombo) {
            DesignsDocument* lastDoc = control->property("ow_last_document").value<DesignsDocument*>();
            return openDocument(lastDoc);
        }
    }
}

void QmlCodeEditorWidget::onFileExplorerFileOpen(const QString& relativePath)
{
    if (m_openDocument->scope == QmlCodeEditorToolBar::Assets)
        return openAssets(relativePath);
    if (m_openDocument->scope == QmlCodeEditorToolBar::Designs)
        return openDesigns(designs(m_openDocument)->control, relativePath);
    if (m_openDocument->scope == QmlCodeEditorToolBar::Others)
        return openOthers(fullPath(m_fileExplorer->rootPath(), relativePath));
}

bool QmlCodeEditorWidget::documentExists(QmlCodeEditorWidget::Document* document) const
{
    for (AssetsDocument* doc : m_assetsDocuments) {
        if (doc == document)
            return true;
    }
    for (DesignsDocument* doc : m_designsDocuments) {
        if (doc == document)
            return true;
    }
    for (OthersDocument* doc : m_OthersDocuments) {
        if (doc == document)
            return true;
    }
    return false;
}

void QmlCodeEditorWidget::openAssets(const QString& relativePath)
{
    if (relativePath.isEmpty())
        return openDocument(g_lastAssetsDocument);
    if (!QFileInfo::exists(fullPath(assetsDir(), relativePath)))
        return (void) (qWarning() << tr("openAssets: File not exists."));
    if (warnIfNotATextFile(fullPath(assetsDir(), relativePath)))
        return;
    if (!assetsExists(relativePath))
        return openDocument(addAssets(relativePath));
    openDocument(getAssets(relativePath));
}

void QmlCodeEditorWidget::openDesigns(Control* control, const QString& relativePath)
{
    if (!control || relativePath.isEmpty())
        return openDocument(g_lastDesignsDocument);
    if (!QFileInfo::exists(fullPath(SaveUtils::toControlThisDir(control->dir()), relativePath)))
        return (void) (qWarning() << tr("openDesigns: File not exists."));
    if (warnIfNotATextFile(fullPath(SaveUtils::toControlThisDir(control->dir()), relativePath)))
        return;
    if (!designsExists(control, relativePath))
        return openDocument(addDesigns(control, relativePath));
    openDocument(getDesigns(control, relativePath));
}

void QmlCodeEditorWidget::openOthers(const QString& fullPath)
{
    if (fullPath.isEmpty())
        return openDocument(g_lastOthersDocument);
    if (!QFileInfo::exists(fullPath))
        return (void) (qWarning() << tr("openOthers: File not exists."));
    if (warnIfNotATextFile(fullPath))
        return;
    if (!othersExists(fullPath))
        return openDocument(addOthers(fullPath));
    openDocument(getOthers(fullPath));
}

bool QmlCodeEditorWidget::assetsExists(const QString& relativePath) const
{
    for (AssetsDocument* document : m_assetsDocuments) {
        if (document->relativePath == relativePath)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::designsExists(Control* control, const QString& relativePath) const
{
    for (DesignsDocument* document : m_designsDocuments) {
        if (document->relativePath == relativePath && control == document->control)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::othersExists(const QString& fullPath) const
{
    for (OthersDocument* document : m_OthersDocuments) {
        if (document->fullPath == fullPath)
            return true;
    }
    return false;
}

QmlCodeEditorWidget::AssetsDocument* QmlCodeEditorWidget::getAssets(const QString& relativePath) const
{
    for (AssetsDocument* document : m_assetsDocuments) {
        if (document->relativePath == relativePath)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::DesignsDocument* QmlCodeEditorWidget::getDesigns(Control* control,
                                                                      const QString& relativePath) const
{
    for (DesignsDocument* document : m_designsDocuments) {
        if (document->relativePath == relativePath && control == document->control)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::OthersDocument*QmlCodeEditorWidget::getOthers(const QString& fullPath) const
{
    for (OthersDocument* document : m_OthersDocuments) {
        if (document->fullPath == fullPath)
            return document;
    }
    return nullptr;
}

QmlCodeEditorWidget::AssetsDocument* QmlCodeEditorWidget::addAssets(const QString& relativePath)
{
    const QString& filePath = assetsDir() + '/' + relativePath;

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("QmlCodeEditorWidget: Cannot open assets file");
        return nullptr; // TODO: Guard against nullptr for those who uses this function
    }

    AssetsDocument* document = new AssetsDocument;
    document->scope = QmlCodeEditorToolBar::Assets;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(filePath);
    document->document->setPlainText(file.readAll());
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);
    file.close();

    m_assetsDocuments.append(document);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Assets) {
        QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
        int i = leftCombo->count();
        leftCombo->addItem(document->relativePath);
        leftCombo->setItemData(i, document->relativePath, Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

    toolBar()->setScopeWide(QmlCodeEditorToolBar::Assets, true);

    return document;
}

QmlCodeEditorWidget::DesignsDocument* QmlCodeEditorWidget::addDesigns(Control* control,
                                                                      const QString& relativePath)
{
    const QString& filePath = fullPath(SaveUtils::toControlThisDir(control->dir()), relativePath);

    QFile file(filePath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("QmlCodeEditorWidget: Cannot open designs file");
        return nullptr; // TODO: Guard against nullptr for those who uses this function
    }

    DesignsDocument* document = new DesignsDocument;
    document->scope = QmlCodeEditorToolBar::Designs;
    document->control = control;
    document->relativePath = relativePath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(filePath);
    document->document->setPlainText(file.readAll());
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);
    file.close();

    m_designsDocuments.append(document);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Designs) {
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

    toolBar()->setScopeWide(QmlCodeEditorToolBar::Designs, true);

    return document;
}

QmlCodeEditorWidget::OthersDocument* QmlCodeEditorWidget::addOthers(const QString& fullPath)
{
    QFile file(fullPath);
    if (!file.open(QFile::ReadOnly)) {
        qWarning("QmlCodeEditorWidget: Cannot open external file");
        return nullptr; // TODO: Guard against nullptr for those who uses this function
    }

    OthersDocument* document = new OthersDocument;
    document->scope = QmlCodeEditorToolBar::Others;
    document->fullPath = fullPath;
    document->document = new QmlCodeDocument(m_codeEditor);
    document->document->setFilePath(fullPath);
    document->document->setPlainText(file.readAll());
    document->document->setModified(false);
    document->textCursor = QTextCursor(document->document);
    file.close();

    m_OthersDocuments.append(document);

    if (toolBar()->scope() == QmlCodeEditorToolBar::Others) {
        QComboBox* leftCombo = toolBar()->combo(QmlCodeEditorToolBar::LeftCombo);
        int i = leftCombo->count();
        leftCombo->addItem(QFileInfo(document->fullPath).fileName());
        leftCombo->setItemData(i, document->fullPath, Qt::ToolTipRole);
        leftCombo->setItemData(i, QVariant::fromValue(document), ComboDataRole::DocumentRole);
    }

    toolBar()->setScopeWide(QmlCodeEditorToolBar::Others, true);

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
    if (document->scope == QmlCodeEditorToolBar::Assets)
        return m_fileExplorer->setRootPath(assetsDir());
    if (document->scope == QmlCodeEditorToolBar::Designs)
        return m_fileExplorer->setRootPath(designsDir(document));
    if (document->scope == QmlCodeEditorToolBar::Others)
        return m_fileExplorer->setRootPath(othersDir(document));
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
    if (mimeData->hasUrls()) {
        e->acceptProposedAction();
        for (const QUrl& url : mimeData->urls()) {
            if (url.isLocalFile())
                openOthers(url.toLocalFile());
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
            || (m_openDocument->scope == QmlCodeEditorToolBar::Designs
                && designs(m_openDocument)->control != designs(document)->control);

    if (refresh) {
        leftCombo->clear();
        rightCombo->clear();
        toolBar()->setScope(scope);

        switch (scope) {
        case QmlCodeEditorToolBar::Assets:
            toolBar()->setHiddenActions(QmlCodeEditorToolBar::RightAction | QmlCodeEditorToolBar::FileActions);
            leftCombo->setToolTip(tr("Relative file path of the open document within the Assets"));
            for (AssetsDocument* doc : m_assetsDocuments) {
                int i = leftCombo->count();
                leftCombo->addItem(modified(doc->document, doc->relativePath));
                leftCombo->setItemData(i, doc->relativePath, Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    leftCombo->setCurrentIndex(i);
            } break;

        case QmlCodeEditorToolBar::Designs:
            toolBar()->setHiddenActions(QmlCodeEditorToolBar::FileActions);
            leftCombo->setToolTip(tr("Control name"));
            rightCombo->setToolTip(tr("Relative file path of the open document within the control"));
            for (Control* control : controls(m_designsDocuments)) {
                int i = leftCombo->count();
                leftCombo->addItem(modifiedControlId(control));
                leftCombo->setItemData(i, control->id() + "::" + control->uid(), Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(control), ComboDataRole::ControlRole);
                if (designs(document)->control == control)
                    leftCombo->setCurrentIndex(i);
            } for (DesignsDocument* doc : m_designsDocuments) {
                if (designs(document)->control == doc->control) {
                    int i = rightCombo->count();
                    rightCombo->addItem(modified(doc->document, doc->relativePath));
                    rightCombo->setItemData(i, doc->relativePath, Qt::ToolTipRole);
                    rightCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                    if (doc == document) {
                        rightCombo->setCurrentIndex(i);
                        designs(document)->control->setProperty("ow_last_document", QVariant::fromValue(doc));
                    }
                }
            } break;

        case QmlCodeEditorToolBar::Others:
            toolBar()->setHiddenActions(QmlCodeEditorToolBar::RightAction);
            leftCombo->setToolTip(tr("File name of the open document"));
            for (OthersDocument* doc : m_OthersDocuments) {
                int i = leftCombo->count();
                leftCombo->addItem(modified(doc->document, QFileInfo(doc->fullPath).fileName()));
                leftCombo->setItemData(i, doc->fullPath, Qt::ToolTipRole);
                leftCombo->setItemData(i, QVariant::fromValue(doc), ComboDataRole::DocumentRole);
                if (doc == document)
                    leftCombo->setCurrentIndex(i);
            } break;
        }
    } else {
        switch (scope) {
        case QmlCodeEditorToolBar::Assets:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, DocumentRole).value<AssetsDocument*>() == document) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } break;

        case QmlCodeEditorToolBar::Others:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, DocumentRole).value<OthersDocument*>() == document) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } break;

        case QmlCodeEditorToolBar::Designs:
            for (int i = 0; i < leftCombo->count(); ++i) {
                if (leftCombo->itemData(i, ControlRole).value<Control*>() == designs(document)->control) {
                    leftCombo->setCurrentIndex(i);
                    break;
                }
            } for (int i = 0; i < rightCombo->count(); ++i) {
                DesignsDocument* doc = rightCombo->itemData(i, DocumentRole).value<DesignsDocument*>();
                if (doc == document) {
                    rightCombo->setCurrentIndex(i);
                    designs(document)->control->setProperty("ow_last_document", QVariant::fromValue(doc));
                    break;
                }
            } break;
        }
    }
}

bool QmlCodeEditorWidget::controlExists(const Control* control)
{
    for (QmlCodeEditorWidget::DesignsDocument* document : m_designsDocuments) {
        if (document->control == control)
            return true;
    }
    return false;
}

bool QmlCodeEditorWidget::controlModified(const Control* control)
{
    Q_ASSERT(controlExists(control));
    for (QmlCodeEditorWidget::DesignsDocument* document : m_designsDocuments) {
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
