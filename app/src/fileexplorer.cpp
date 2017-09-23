#include <fileexplorer.h>
#include <fit.h>
#include <css.h>
#include <filelist.h>
#include <filemanager.h>

#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>

#define COLOR_BACKGROUND (QColor("#F3F7FA"))

using namespace Fit;

class FileExplorerPrivate : public QObject
{
        Q_OBJECT
    public:
        FileExplorerPrivate(FileExplorer* parent);

    private slots:
        void handleUpButtonClicked();
        void handleHomeButtonClicked();
        void handleCopyButtonClicked();
        void handleDeleteButtonClicked();
        void handleNewFileButtonClicked();
        void handleNewFolderButtonClicked();
        void handleDownloadButtonClicked();
        void handleFileListSelectionChanged();

    public:
        FileExplorer* parent;
        QVBoxLayout vBoxLayout;
        QToolBar toolbar;
        QToolButton upButton;
        QToolButton homeButton;
        QToolButton copyButton;
        QToolButton deleteButton;
        QToolButton newFileButton;
        QToolButton newFolderButton;
        QToolButton downloadFileButton;
        FileList fileList;

};

FileExplorerPrivate::FileExplorerPrivate(FileExplorer* parent)
    : QObject(parent)
    , parent(parent)
    , vBoxLayout(parent)
{
    vBoxLayout.setContentsMargins(0, 0, 0, 0);
    vBoxLayout.setSpacing(0);
    vBoxLayout.addWidget(&toolbar);
    vBoxLayout.addWidget(&fileList);

    deleteButton.setDisabled(true);

    upButton.setCursor(Qt::PointingHandCursor);
    homeButton.setCursor(Qt::PointingHandCursor);
    copyButton.setCursor(Qt::PointingHandCursor);
    deleteButton.setCursor(Qt::PointingHandCursor);
    newFileButton.setCursor(Qt::PointingHandCursor);
    newFolderButton.setCursor(Qt::PointingHandCursor);
    downloadFileButton.setCursor(Qt::PointingHandCursor);

    upButton.setToolTip("Go up.");
    homeButton.setToolTip("Go home.");
    copyButton.setToolTip("Copy file/folder.");
    deleteButton.setToolTip("Delete file/folder.");
    newFileButton.setToolTip("New file.");
    newFolderButton.setToolTip("New folder.");
    downloadFileButton.setToolTip("Download file from url.");

    upButton.setIcon(QIcon(":/resources/images/up.png"));
    homeButton.setIcon(QIcon(":/resources/images/home.png"));
    copyButton.setIcon(QIcon(":/resources/images/copy.png"));
    deleteButton.setIcon(QIcon(":/resources/images/delete.png"));
    newFileButton.setIcon(QIcon(":/resources/images/newfile.png"));
    newFolderButton.setIcon(QIcon(":/resources/images/newfolder.png"));
    downloadFileButton.setIcon(QIcon(":/resources/images/downloadfile.png"));

    connect(&upButton, SIGNAL(clicked(bool)), SLOT(handleUpButtonClicked()));
    connect(&homeButton, SIGNAL(clicked(bool)), SLOT(handleHomeButtonClicked()));
    connect(&copyButton, SIGNAL(clicked(bool)), SLOT(handleCopyButtonClicked()));
    connect(&deleteButton, SIGNAL(clicked(bool)), SLOT(handleDeleteButtonClicked()));
    connect(&newFileButton, SIGNAL(clicked(bool)), SLOT(handleNewFileButtonClicked()));
    connect(&newFolderButton, SIGNAL(clicked(bool)), SLOT(handleNewFolderButtonClicked()));
    connect(&downloadFileButton, SIGNAL(clicked(bool)), SLOT(handleDownloadButtonClicked()));
    connect(fileList.selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            SLOT(handleFileListSelectionChanged()));

    toolbar.setStyleSheet(CSS::DesignerToolbar);
    toolbar.setIconSize(QSize(fit(14), fit(14)));
    toolbar.setFixedHeight(fit(26));
    toolbar.addWidget(&upButton);
    toolbar.addWidget(&homeButton);
    toolbar.addSeparator();
    toolbar.addWidget(&copyButton);
    toolbar.addWidget(&deleteButton);
    toolbar.addSeparator();
    toolbar.addWidget(&newFileButton);
    toolbar.addWidget(&newFolderButton);
    toolbar.addWidget(&downloadFileButton);

    QPalette p(fileList.palette());
    p.setColor(QPalette::Background, COLOR_BACKGROUND);
    p.setColor(QPalette::Base, COLOR_BACKGROUND);
    fileList.setPalette(p);
    fileList.setAutoFillBackground(true);
}

void FileExplorerPrivate::handleUpButtonClicked()
{
    auto up = dname(fileList.currentPath());
    auto rootPath = fileList.fileModel()->rootPath();
    if (up.size() > rootPath.size() || up == rootPath)
        fileList.goPath(up);
}

void FileExplorerPrivate::handleHomeButtonClicked()
{
    fileList.goPath(fileList.fileModel()->rootPath());
}

void FileExplorerPrivate::handleCopyButtonClicked()
{
    auto _index = fileList.currentIndex();
    auto index = fileList.fileModel()->index(_index.row(), 0, fileList.rootIndex());
    auto fileName = fileList.fileModel()->fileName(index);
    auto filePath = fileList.fileModel()->filePath(index);

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
            if (fileList.fileModel()->isDir(index)) {
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

void FileExplorerPrivate::handleDeleteButtonClicked()
{
    auto _index = fileList.currentIndex();
    auto index = fileList.fileModel()->index(_index.row(), 0, fileList.rootIndex());
    auto fileName = fileList.fileModel()->fileName(index);
    auto filePath = fileList.fileModel()->filePath(index);

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
            rm(filePath);
            break;
        } default: {
            // Do nothing
            break;
        }
    }
}

void FileExplorerPrivate::handleNewFileButtonClicked()
{

}

void FileExplorerPrivate::handleNewFolderButtonClicked()
{

}

void FileExplorerPrivate::handleDownloadButtonClicked()
{

}

void FileExplorerPrivate::handleFileListSelectionChanged()
{
    auto _index = fileList.currentIndex();
    auto index = fileList.fileModel()->index(_index.row(), 0, fileList.rootIndex());
    deleteButton.setEnabled(index.isValid());
    copyButton.setEnabled(index.isValid());
}

FileExplorer::FileExplorer(QWidget *parent)
    : QWidget(parent)
    , _d(new FileExplorerPrivate(this))
{
    setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Background, COLOR_BACKGROUND);
    p.setColor(QPalette::Base, COLOR_BACKGROUND);
    setPalette(p);
}

#include "fileexplorer.moc"


