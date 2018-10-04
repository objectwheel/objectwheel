#include <utilityfunctions.h>
#include <saveutils.h>
#include <filemanager.h>
#include <delayer.h>
#include <offlinestorage.h>
#include <utils/utilsicons.h>

#include <QFileInfo>
#include <QMessageBox>
#include <QQmlEngine>
#include <QTextDocument>
#include <QRegularExpression>
#include <QTextCursor>
#include <QProgressDialog>
#include <QtConcurrent>
#include <QTreeView>
#include <QScreen>
#include <QWindow>
#include <QAbstractButton>

namespace {
QString g_projectDirectory;
}

void UtilityFunctions::registerGlobalPath(const QString& projectDirectory)
{
    g_projectDirectory = projectDirectory;
    qmlRegisterSingletonType("Objectwheel.GlobalResources", 1, 0, "GlobalResources",
                             [] (QQmlEngine* engine, QJSEngine* jsEngine) -> QJSValue {
        Q_UNUSED(engine)
        QJSValue globalPath = jsEngine->newObject();
        globalPath.setProperty("path", SaveUtils::toGlobalDir(g_projectDirectory));
        globalPath.setProperty("url", jsEngine->toScriptValue(
                                   QUrl::fromLocalFile(SaveUtils::toGlobalDir(g_projectDirectory))));
        return globalPath;
    });
}

void UtilityFunctions::registerOfflineStorage()
{
    qmlRegisterSingletonType<OfflineStorage>("Objectwheel.OfflineStorage", 1, 0, "OfflineStorage",
                                             [] (QQmlEngine* engine, QJSEngine* jsEngine) -> QObject* {
        Q_UNUSED(jsEngine)
        return new OfflineStorage(engine);
    });
}

void UtilityFunctions::trimCommentsAndStrings(QTextDocument* document)
{
    QRegularExpression exp("(([\"'])(?:\\\\[\\s\\S]|.)*?\\2|\\/(?![*\\/])(?:\\\\.|\\[(?:\\\\.|.)\\]|.)*?\\/)|\\/\\/.*?$|\\/\\*[\\s\\S]*?\\*\\/",
                           QRegularExpression::MultilineOption); // stackoverflow.com/q/24518020

    QRegularExpressionMatchIterator i = exp.globalMatch(document->toPlainText());
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString capturedText = match.captured();
        int start = match.capturedStart();
        int end = match.capturedEnd();

        capturedText.replace(QRegularExpression("[^\\n\\r\\'\\\"]", QRegularExpression::MultilineOption), "x");

        QTextCursor cursor(document);
        cursor.setPosition(start);
        cursor.setPosition(end, QTextCursor::KeepAnchor);
        cursor.insertText(capturedText);
    }
}

QWidget* UtilityFunctions::createSpacerWidget(Qt::Orientation orientation)
{
    auto spacer = new QWidget;
    spacer->setSizePolicy((orientation & Qt::Horizontal)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred,
                          (orientation & Qt::Vertical)
                          ? QSizePolicy::Expanding : QSizePolicy::Preferred);
    return spacer;
}

void UtilityFunctions::copyFiles(const QString& rootPath, const QList<QUrl>& urls, QWidget* parent)
{
    QProgressDialog progress("Copying files...", "Abort Copy", 0, urls.size(), parent);
    progress.setWindowModality(Qt::NonModal);
    progress.open();
    Delayer::delay(100);

    bool askForOverwrite = true;

    for (int i = 0; i < urls.size(); i++) {
        const QUrl& url = urls.at(i);

        progress.setValue(i);

        if (progress.wasCanceled())
            break;

        if (url.isEmpty())
            continue;

        if (!url.isValid())
            continue;

        if (!url.isLocalFile())
            continue;

        const QString& path = QFileInfo(url.toLocalFile()).canonicalFilePath();
        const QString& fileName = fname(path);
        const QString& destPath = rootPath + separator() + fileName;

        if (exists(destPath)) {
            if (askForOverwrite) {
                int ret = QMessageBox::question(
                            parent,
                            QObject::tr("File or folder exists"),
                            QObject::tr("File or folder exists. "
                                        "Would you like to overwrite following file/folder: ") + fileName,
                            QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll| QMessageBox::Abort,
                            QMessageBox::No);
                if (ret == QMessageBox::Yes) {
                    rm(destPath);
                } else if (ret == QMessageBox::No) {
                    continue;
                } else if (ret == QMessageBox::YesToAll) {
                    askForOverwrite = false;
                    rm(destPath);
                } else {
                    break;
                }
            } else {
                rm(destPath);
            }
        }

        QFuture<void> future = QtConcurrent::run(qOverload<const QString&, const QString&, bool, bool>(&cp),
                                                 path, rootPath, false, false);
        Delayer::delay(std::bind(&QFuture<void>::isRunning, &future));
    }

    progress.setValue(urls.size());
    Delayer::delay(100);
}

void UtilityFunctions::expandUpToRoot(QTreeView* view, const QModelIndex& index, const QModelIndex& rootIndex)
{
    if (!index.isValid())
        return;

    if (index == rootIndex)
        return;

    view->expand(index);

    expandUpToRoot(view, index.parent(), rootIndex);
}

QWindow* UtilityFunctions::window(const QWidget* widget)
{
    Q_ASSERT(widget);
    QWindow* winHandle = widget->windowHandle();
    if (!winHandle) {
        if (const QWidget* nativeParent = widget->nativeParentWidget())
            winHandle = nativeParent->windowHandle();
    }
    Q_ASSERT(winHandle);
    return winHandle;
}

void UtilityFunctions::centralizeWidget(QWidget* widget)
{
    widget->setGeometry(QStyle::alignedRect(widget->layoutDirection(), Qt::AlignCenter, widget->size(),
                                            window(widget)->screen()->availableGeometry()));
}

void UtilityFunctions::adjustFontPixelSize(QWidget* widget, int advance)
{
    QFont font(widget->font());
    font.setPixelSize(font.pixelSize() + advance);
    widget->setFont(font);
}

void UtilityFunctions::adjustFontWeight(QWidget* widget, QFont::Weight weight)
{
    QFont font(widget->font());
    font.setWeight(weight);
    widget->setFont(font);
}

bool UtilityFunctions::hasHover(const QWidget* widget) // FIXME: This is a workaround for QTBUG-44400
{
    return widget->isVisible()
            && widget->isEnabled()
            && widget->rect().contains(widget->mapFromGlobal(QCursor::pos()));
}

QRectF UtilityFunctions::verticalAlignedRect(const QSizeF& size, const QRectF& rect, qreal left)
{
    QRectF ret({0, 0}, size);
    ret.moveCenter(rect.center());
    if (left >= 0)
        ret.moveLeft(left);
    return ret;
}

QRectF UtilityFunctions::horizontalAlignedRect(const QSizeF& size, const QRectF& rect, qreal top)
{
    QRectF ret({0, 0}, size);
    ret.moveCenter(rect.center());
    if (top >= 0)
        ret.moveTop(top);
    return ret;
}

QPixmap UtilityFunctions::pixmap(QAbstractButton* button, const QIcon& icon, const QSizeF& size)
{
    return icon.pixmap(window(button), size.toSize(), !button->isEnabled()
                       ? QIcon::Disabled
                       : (button->isDown() ? QIcon::Active : QIcon::Normal));
}