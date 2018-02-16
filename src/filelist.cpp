#include <filelist.h>
#include <fit.h>
#include <css.h>
#include <global.h>
#include <filemanager.h>
#include <delayer.h>

#include <QDebug>
#include <QMouseEvent>
#include <QHeaderView>
#include <QLabel>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QGraphicsBlurEffect>
#include <QScrollBar>
#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QtWidgets>
#include <QtConcurrent>

#define pS (qApp->primaryScreen())

QLabel* dropLabel;
QGraphicsBlurEffect* blurEffect;

struct DropSettings {
  QPixmap ao, a, b;
  QRectF r, ra;
}* dbs; // Drop box settings

FileList::FileList(QWidget *parent) : QTreeView(parent)
{
    _fileModel.setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    setItemsExpandable(false);
    setRootIsDecorated(false);
    setSortingEnabled(true);
    header()->setSectionsMovable(false);

    verticalScrollBar()->setStyleSheet(CSS::ScrollBar);
    horizontalScrollBar()->setStyleSheet(CSS::ScrollBarH);

    _filterProxyModel.setDynamicSortFilter(true);
    _filterProxyModel.setFilterKeyColumn(0);
    _filterProxyModel.setSourceModel(&_fileModel);

    blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
    blurEffect->setEnabled(false);
    blurEffect->setBlurRadius(fit::fx(70));
    viewport()->setGraphicsEffect(blurEffect);

    dbs = new DropSettings;
    dbs->ao.load(":/resources/images/droparrow.png");
    dbs->r.setSize({fit::fx(100), fit::fx(100)});

    dropLabel = new QLabel(this);
    dropLabel->setHidden(true);
    dropLabel->setAlignment(Qt::AlignCenter);
    dropLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    dropLabel->raise();

    setAcceptDrops(true);
    setAttribute(Qt::WA_MacShowFocusRect, false);
}

FileList::~FileList()
{
    delete dbs;
}

void FileList::mouseDoubleClickEvent(QMouseEvent* event)
{
    auto _index = _filterProxyModel.mapToSource(indexAt(event->pos()));
    auto index = _fileModel.index(_index.row(), 0, _filterProxyModel.
                                  mapToSource(rootIndex()));
    if (!index.isValid())
        return;
    if (_fileModel.isDir(index))
        setRootIndex(_filterProxyModel.mapFromSource(index));
    else
        emit fileOpened(_fileModel.filePath(index));
}

FileFilterProxyModel* FileList::filterProxyModel()
{
    return &_filterProxyModel;
}

QFileSystemModel* FileList::fileModel()
{
    return &_fileModel;
}

QString FileList::currentPath() const
{
    return _fileModel.filePath(
      _filterProxyModel.mapToSource(rootIndex()));
}

void FileList::goPath(const QString& path)
{
    setRootIndex(_filterProxyModel.
      mapFromSource(_fileModel.index(path)));
}

void FileList::handleDrop(const QList<QUrl>& urls)
{
    auto showMsgBox = true;

    QProgressDialog progress("Copying files...", "Abort Copy", 0, urls.size(), this);
    progress.setWindowModality(Qt::NonModal);
    progress.open();
    Delayer::delay(100);

    for (int i = 0; i < urls.size(); i++) {
        progress.setValue(i);

        if (progress.wasCanceled())
            break;

        auto url = urls.at(i);
        if (!url.isLocalFile() ||
          !url.isValid() || url.isEmpty()) {
            Q_ASSERT(0);
            continue;
        }

        auto path = url.toLocalFile();
        if (path.at(path.size() - 1) == '\\' || path.at(path.size() - 1) == '/')
            path.remove(path.size() - 1, 1);

        if (exists(currentPath() + separator() + fname(path))) {
            if (showMsgBox) {
                QMessageBox msgbox;
                msgbox.setText("It already exists. Would you like to overwrite following file/folder?");
                msgbox.setInformativeText(fname(path));
                msgbox.setIcon(QMessageBox::Icon::Question);
                msgbox.addButton(QMessageBox::Yes);
                msgbox.addButton(QMessageBox::No);
                msgbox.addButton(QMessageBox::YesToAll);
                msgbox.addButton(QMessageBox::Abort);
                msgbox.setDefaultButton(QMessageBox::No);
                msgbox.setWindowModality(Qt::ApplicationModal);

                int ret = msgbox.exec();
                if (ret == QMessageBox::Yes) {
                    rm(currentPath() + separator() + fname(path));
                } else if (ret == QMessageBox::No) {
                    continue;
                } else if (ret == QMessageBox::YesToAll) {
                    showMsgBox = false;
                } else {
                    break;
                }
            } else {
                rm(currentPath() + separator() + fname(path));
            }
        }
        auto future = QtConcurrent::run((void (*)(const QString&,
          const QString&, bool, bool))&cp, path, currentPath(), false, false);
        Delayer::delay(std::bind(&QFuture<void>::isRunning, &future));
    }
    progress.setValue(urls.size());
    Delayer::delay(100);
}

void FileList::dragEnterEvent(QDragEnterEvent* event)
{
    if (event->mimeData()->hasUrls() &&
     !(event->mimeData()->hasText() &&
      event->mimeData()->text() == TOOL_KEY)) {
         event->accept();
         dropLabel->setVisible(true);
         blurEffect->setEnabled(true);
    }
}

void FileList::dropEvent(QDropEvent* event)
{
    if (event->mimeData()->hasUrls() &&
     !(event->mimeData()->hasText() &&
      event->mimeData()->text() == TOOL_KEY)) {
        event->accept();
        handleDrop(event->mimeData()->urls());
    }
    dropLabel->setHidden(true);
    blurEffect->setEnabled(false);
}

void FileList::dragMoveEvent(QDragMoveEvent* event)
{
    event->accept();
}

void FileList::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
    dropLabel->setHidden(true);
    blurEffect->setEnabled(false);
}

void FileList::resizeEvent(QResizeEvent* event)
{
    QTreeView::resizeEvent(event);
    dropLabel->setGeometry(viewport()->geometry());

    QPixmap bn(dropLabel->size() * pS->devicePixelRatio());
    bn.setDevicePixelRatio(pS->devicePixelRatio());
    bn.fill("#15000000");
    dbs->b = bn;
    dbs->r.moveCenter(dropLabel->rect().center());
    dbs->ra = dbs->r.adjusted(fit::fx(25), fit::fx(17), -fit::fx(25), -fit::fx(33));
    dbs->a = dbs->ao.scaled((dbs->ra.size() * pS->devicePixelRatio()).toSize());

    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(fit::fx(3));
    pen.setColor("#b4b8bb");

    QPainter painter(&dbs->b);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(pen);

    painter.drawRoundedRect(dbs->r, fit::fx(12), fit::fx(12));
    painter.drawPixmap(dbs->ra, dbs->a, dbs->a.rect());

    pen.setWidthF(fit::fx(1));
    pen.setColor("#b0b4b7");
    painter.setPen(pen);

    painter.drawText(dbs->r.adjusted(0, fit::fx(10), 0, -fit::fx(10)), "Drop Here",
      QTextOption(Qt::AlignHCenter | Qt::AlignBottom));
    painter.end();
    dropLabel->setPixmap(dbs->b);
}
