#include <helpviewer.h>
#include <QFileInfo>
#include <QUrl>
#include <QApplication>
#include <QDesktopServices>
#include <QMouseEvent>
#include <QHelpEngine>

struct ExtensionMap {
    const char *extension;
    const char *mimeType;
} extensionMap[] = {
    {".bmp", "image/bmp"},
    {".css", "text/css"},
    {".gif", "image/gif"},
    {".html", "text/html"},
    {".htm", "text/html"},
    {".ico", "image/x-icon"},
    {".jpeg", "image/jpeg"},
    {".jpg", "image/jpeg"},
    {".js", "application/x-javascript"},
    {".mng", "video/x-mng"},
    {".pbm", "image/x-portable-bitmap"},
    {".pgm", "image/x-portable-graymap"},
    {".pdf", "application/pdf"},
    {".png", "image/png"},
    {".ppm", "image/x-portable-pixmap"},
    {".rss", "application/rss+xml"},
    {".svg", "image/svg+xml"},
    {".svgz", "image/svg+xml"},
    {".text", "text/plain"},
    {".tif", "image/tiff"},
    {".tiff", "image/tiff"},
    {".txt", "text/plain"},
    {".xbm", "image/x-xbitmap"},
    {".xml", "text/xml"},
    {".xpm", "image/x-xpm"},
    {".xsl", "text/xsl"},
    {".xhtml", "application/xhtml+xml"},
    {".wml", "text/vnd.wap.wml"},
    {".wmlc", "application/vnd.wap.wmlc"},
    {"about:blank", 0},
    {0, 0}
};

HelpViewer::HelpViewer(QWidget *parent) : QWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);
}

void HelpViewer::setActionVisible(Action action, bool visible)
{
    if (visible)
        m_visibleActions |= Actions(action);
    else
        m_visibleActions &= ~Actions(action);
}

bool HelpViewer::isActionVisible(HelpViewer::Action action)
{
    return (m_visibleActions & Actions(action)) != 0;
}

bool HelpViewer::isLocalUrl(const QUrl &url)
{
    return url.scheme() == "about" // "No documenation available"
            || url.scheme() == "qthelp";
}

bool HelpViewer::canOpenPage(const QString &url)
{
    return !mimeFromUrl(url).isEmpty();
}

QString HelpViewer::mimeFromUrl(const QUrl &url)
{
    const QString &path = url.path();
    const int index = path.lastIndexOf(QLatin1Char('.'));
    const QByteArray &ext = path.mid(index).toUtf8().toLower();

    const ExtensionMap *e = extensionMap;
    while (e->extension) {
        if (ext == e->extension)
            return QLatin1String(e->mimeType);
        ++e;
    }
    return QString();
}

void HelpViewer::home()
{
    static const QString url = "qthelp://org.qt-project.qtdoc.5101/qtdoc/index.html";
    setSource(url);
}

void HelpViewer::slotLoadStarted()
{
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}
void HelpViewer::slotLoadFinished()
{
    QApplication::restoreOverrideCursor();
    emit sourceChanged(source());
    emit loadFinished();
}

bool HelpViewer::handleForwardBackwardMouseButtons(QMouseEvent *event)
{
    if (event->button() == Qt::XButton1) {
        backward();
        return true;
    }
    if (event->button() == Qt::XButton2) {
        forward();
        return true;
    }

    return false;
}
