#include <fileexplorer.h>
#include <filelist.h>
#include <filemanager.h>
#include <utilsicons.h>
#include <transparentstyle.h>

#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QInputDialog>
#include <QtNetwork>

class FileExplorerPrivate : public QObject
{
        Q_OBJECT
    public:
        FileExplorerPrivate(FileExplorer* parent);

    public slots:
        void onUpButtonClicked();
        void onHomeButtonClicked();
        void onCopyButtonClicked();
        void onDeleteButtonClicked();
        void onNewFileButtonClicked();
        void onRenameButtonClicked();
        void onNewFolderButtonClicked();
        void onDownloadButtonClicked();
        void onFileListSelectionChanged();

    public:
        FileExplorer* parent;
        QVBoxLayout* vBoxLayout;
        QToolBar* toolbar;
        QToolButton* upButton;
        QToolButton* homeButton;
        QToolButton* copyButton;
        QToolButton* deleteButton;
        QToolButton* renameButton;
        QToolButton* newFileButton;
        QToolButton* newFolderButton;
        QToolButton* downloadFileButton;
        FileList* fileList;
        QMetaObject::Connection previousSelectionModelConnection;
};

FileExplorerPrivate::FileExplorerPrivate(FileExplorer* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(new QVBoxLayout(parent))
    , toolbar(new QToolBar)
    , upButton(new QToolButton)
    , homeButton(new QToolButton)
    , copyButton(new QToolButton)
    , deleteButton(new QToolButton)
    , renameButton(new QToolButton)
    , newFileButton(new QToolButton)
    , newFolderButton(new QToolButton)
    , downloadFileButton(new QToolButton)
    , fileList(new FileList)
{
    vBoxLayout->setContentsMargins(0, 0, 0, 0);
    vBoxLayout->setSpacing(0);
    vBoxLayout->addWidget(toolbar);
    vBoxLayout->addWidget(fileList);

    deleteButton->setDisabled(true);
    copyButton->setDisabled(true);

    upButton->setCursor(Qt::PointingHandCursor);
    homeButton->setCursor(Qt::PointingHandCursor);
    copyButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setCursor(Qt::PointingHandCursor);
    renameButton->setCursor(Qt::PointingHandCursor);
    newFileButton->setCursor(Qt::PointingHandCursor);
    newFolderButton->setCursor(Qt::PointingHandCursor);
    downloadFileButton->setCursor(Qt::PointingHandCursor);

    upButton->setToolTip("Go up.");
    homeButton->setToolTip("Go home.");
    copyButton->setToolTip("Copy file/folder.");
    deleteButton->setToolTip("Delete file/folder.");
    renameButton->setToolTip("Rename file/folder.");
    newFileButton->setToolTip("New file.");
    newFolderButton->setToolTip("New folder.");
    downloadFileButton->setToolTip("Download file from url.");

    upButton->setIcon(QIcon(":/images/up.png"));
    homeButton->setIcon(Utils::Icons::HOME_TOOLBAR.icon());
    copyButton->setIcon(Utils::Icons::COPY_TOOLBAR.icon()); // TODO: Fix icons
    deleteButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    renameButton->setIcon(QIcon(":/images/rename.png"));
    newFileButton->setIcon(QIcon(":/images/newfile.png"));
    newFolderButton->setIcon(QIcon(":/images/newfolder.png"));
    downloadFileButton->setIcon(QIcon(":/images/downloadfile.png"));

    connect(upButton, SIGNAL(clicked(bool)), SLOT(onUpButtonClicked()));
    connect(homeButton, SIGNAL(clicked(bool)), SLOT(onHomeButtonClicked()));
    connect(copyButton, SIGNAL(clicked(bool)), SLOT(onCopyButtonClicked()));
    connect(deleteButton, SIGNAL(clicked(bool)), SLOT(onDeleteButtonClicked()));
    connect(renameButton, SIGNAL(clicked(bool)), SLOT(onRenameButtonClicked()));
    connect(newFileButton, SIGNAL(clicked(bool)), SLOT(onNewFileButtonClicked()));
    connect(newFolderButton, SIGNAL(clicked(bool)), SLOT(onNewFolderButtonClicked()));
    connect(downloadFileButton, SIGNAL(clicked(bool)), SLOT(onDownloadButtonClicked()));

    TransparentStyle::attach(toolbar);

    upButton->setFixedHeight(22);
    homeButton->setFixedHeight(22);
    copyButton->setFixedHeight(22);
    deleteButton->setFixedHeight(22);
    renameButton->setFixedHeight(22);
    newFileButton->setFixedHeight(22);
    newFolderButton->setFixedHeight(22);
    downloadFileButton->setFixedHeight(22);

    toolbar->setFixedHeight(24);
    toolbar->addWidget(upButton);
    toolbar->addWidget(homeButton);
    toolbar->addSeparator();
    toolbar->addWidget(copyButton);
    toolbar->addWidget(deleteButton);
    toolbar->addSeparator();
    toolbar->addWidget(renameButton);
    toolbar->addWidget(newFileButton);
    toolbar->addWidget(newFolderButton);
    toolbar->addWidget(downloadFileButton);
}

void FileExplorerPrivate::onUpButtonClicked()
{
    auto up = dname(fileList->currentPath());
    auto rootPath = fileList->fileModel()->rootPath();
    if (up.size() > rootPath.size() || up == rootPath)
        fileList->goPath(up);
}

void FileExplorerPrivate::onHomeButtonClicked()
{
    fileList->goPath(fileList->fileModel()->rootPath());
}

void FileExplorerPrivate::onCopyButtonClicked()
{
    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    auto fileName = fileList->fileModel()->fileName(index);
    auto filePath = fileList->fileModel()->filePath(index);

    if (!index.isValid() || fileName.isEmpty() || filePath.isEmpty())
        return;

    QMessageBox box;
    box.setText("<b>Do you want to make a copy of following file/folder.</b>");
    box.setInformativeText("<b>Name: </b>" + fileName);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    box.setIcon(QMessageBox::Warning);
    const int ret = box.exec();

    switch (ret) {
        case QMessageBox::Yes: {
            if (fileList->fileModel()->isDir(index)) {
                auto copyName = fileName;
                for (int i = 1; exists(dname(filePath) + separator() + copyName); i++)
                    copyName = fileName + QString::number(i);
                mkdir(dname(filePath) + separator() + copyName);
                cp(filePath, dname(filePath) + separator() + copyName, true);
            } else {
                QString copyName = fileName;
                auto sfx = suffix(fileName);
                if (!sfx.isEmpty())
                    copyName.remove(copyName.lastIndexOf(sfx) - 1, sfx.size() + 1);

                if (sfx.isEmpty()) {
                    for (int i = 1; exists(dname(filePath) + separator() + copyName); i++)
                        copyName = fileName + QString::number(i);
                    QFile::copy(filePath, dname(filePath) + separator() + copyName);
                } else {
                    auto copy = copyName;
                    for (int i = 1; exists(dname(filePath) + separator() + copyName + "." + sfx); i++)
                        copyName = copy + QString::number(i);
                    QFile::copy(filePath, dname(filePath) + separator() + copyName + "." + sfx);
                }
            }
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void FileExplorerPrivate::onDeleteButtonClicked()
{
    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    auto fileName = fileList->fileModel()->fileName(index);
    auto filePath = fileList->fileModel()->filePath(index);

    if (fileName.startsWith("_") || fileName == "icon.png" || fileName == "main.qml")
        return;

    if (!index.isValid() || fileName.isEmpty() || filePath.isEmpty())
        return;

    QMessageBox box;
    box.setText("<b>Do you want to delete following file/folder.</b>");
    box.setInformativeText("<b>Name: </b>" + fileName);
    box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    box.setDefaultButton(QMessageBox::No);
    box.setIcon(QMessageBox::Warning);
    const int ret = box.exec();

    switch (ret) {
        case QMessageBox::Yes: {
            if (rm(filePath))
                emit parent->fileDeleted(filePath);
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void FileExplorerPrivate::onRenameButtonClicked()
{
    bool ok;
    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    auto filePath = fileList->fileModel()->filePath(index);
    auto fileName = fileList->fileModel()->fileName(index);

    if (fileName.startsWith("_") || fileName == "icon.png" || fileName == "main.qml")
        return;

    QString text = QInputDialog::getText(parent, tr("Rename file/folder"),
                                         tr("New name:"), QLineEdit::Normal,
                                         fileName, &ok);

    if (text.startsWith("_") || text == "icon.png" || text == "main.qml")
        return;

    if (index.isValid() && ok && !text.isEmpty() && text != fileName)
        if (rn(filePath, dname(filePath) + separator() + text))
            emit parent->fileRenamed(filePath, dname(filePath) + separator() + text);
}

void FileExplorerPrivate::onNewFileButtonClicked()
{
    bool ok;
    auto index = fileList->filterProxyModel()->mapToSource(fileList->rootIndex());
    auto path = fileList->fileModel()->filePath(index);
    QString text = QInputDialog::getText(parent, tr("Create new file"),
                                         tr("File name:"), QLineEdit::Normal,
                                         QString(), &ok);

    if (text.startsWith("_") || text == "icon.png" || text == "main.qml")
        return;

    if (index.isValid() && ok && !text.isEmpty() && !exists(path + separator() + text))
        mkfile(path + separator() + text);
}


void FileExplorerPrivate::onNewFolderButtonClicked()
{
    bool ok;
    auto index = fileList->filterProxyModel()->mapToSource(fileList->rootIndex());
    auto path = fileList->fileModel()->filePath(index);
    QString text = QInputDialog::getText(parent, tr("Create new folder"),
                                         tr("Folder name:"), QLineEdit::Normal,
                                         QString(), &ok);

    if (text.startsWith("_") || text == "icon.png" || text == "main.qml")
        return;

    if (index.isValid() && ok && !text.isEmpty() && !exists(path + separator() + text))
        mkdir(path + separator() + text);
}

void FileExplorerPrivate::onDownloadButtonClicked()
{
    bool ok, ok_2;
    auto index = fileList->filterProxyModel()->mapToSource(fileList->rootIndex());
    auto path = fileList->fileModel()->filePath(index);
    QString text = QInputDialog::getText(parent, tr("Download file"),
                                         tr("Url:"), QLineEdit::Normal,
                                         QString(), &ok);

    if (!ok || text.isEmpty())
        return;

    QString text_2 = QInputDialog::getText(parent, tr("Download file"),
                                         tr("File name:"), QLineEdit::Normal,
                                         QString(), &ok_2);

    if (text_2.startsWith("_") || text_2 == "icon.png" || text_2 == "main.qml")
        return;

    if (index.isValid() && ok_2 && !text_2.isEmpty() && !exists(path + separator() + text_2)) {
        const auto& data = dlfile(text);
        if (data.isEmpty())
            return;
        wrfile(path + separator() + text_2, data);
    }
}

void FileExplorerPrivate::onFileListSelectionChanged()
{
    auto _index = fileList->filterProxyModel()->mapToSource(fileList->currentIndex());
    auto index = fileList->fileModel()->index(_index.row(), 0, fileList->
                 filterProxyModel()->mapToSource(fileList->rootIndex()));
    deleteButton->setEnabled(index.isValid());
    copyButton->setEnabled(index.isValid());
}

FileExplorer::FileExplorer(QWidget *parent) : QFrame(parent)
    , _d(new FileExplorerPrivate(this))
{
    QPalette p(palette());
    p.setColor(QPalette::Background, "#f0f0f0");
    p.setColor(QPalette::Base, "#f0f0f0");
    setPalette(p);

    QPalette p2(_d->fileList->palette());
    p2.setColor(QPalette::Background, Qt::white);
    p2.setColor(QPalette::Base, Qt::white);
    _d->fileList->setPalette(p2);

    setFrameShadow(QFrame::Plain);
    setFrameShape(QFrame::StyledPanel);

    connect(_d->fileList, SIGNAL(fileOpened(QString)), SIGNAL(fileOpened(QString)));
}

void FileExplorer::setRootPath(const QString& rootPath)
{
    _d->fileList->fileModel()->setRootPath(rootPath);
    _d->fileList->setModel(_d->fileList->filterProxyModel());
    auto index = _d->fileList->fileModel()->index(rootPath);
    if (!index.isValid()) {
        _d->fileList->setModel(nullptr);
        _d->fileList->fileModel()->setRootPath(QDir::currentPath());
    } else {
        _d->fileList->setRootIndex(_d->fileList->filterProxyModel()->mapFromSource(index));
    }
    disconnect(_d->previousSelectionModelConnection);
    _d->previousSelectionModelConnection = connect(_d->fileList->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                                                   _d, SLOT(onFileListSelectionChanged()));
    _d->onFileListSelectionChanged();
}

QSize FileExplorer::sizeHint() const
{
    return QSize(200, 680);
}

QString FileExplorer::rootPath() const
{
    return _d->fileList->fileModel()->rootPath();
}

void FileExplorer::sweep()
{
    // TODO:
}

#include "fileexplorer.moc"
