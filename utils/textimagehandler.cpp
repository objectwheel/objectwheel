#include <textimagehandler.h>
#include <paintutils.h>

#include <QThread>
#include <QPainter>
#include <QFileInfo>
#include <QApplication>
#include <QWidget>

TextImageHandler::TextImageHandler(QObject* parent) : QTextImageHandler(parent)
{
}

void TextImageHandler::drawObject(QPainter* p, const QRectF& r, QTextDocument* d, int o, const QTextFormat& f)
{
    const QString& fileName = f.toImageFormat().name();
    if (QString::compare(QFileInfo(fileName).suffix(),
                         QStringLiteral("svg"), Qt::CaseInsensitive) == 0) {
        if (QCoreApplication::instance()->thread() == QThread::currentThread()) {
            const QPixmap pixmap = PaintUtils::pixmap(fileName, r.size().toSize(),
                                                      dynamic_cast<QWidget*>(p->device()));
            p->drawPixmap(r, pixmap, pixmap.rect());
            return;
        }
    }
    QTextImageHandler::drawObject(p, r, d, o, f);
}