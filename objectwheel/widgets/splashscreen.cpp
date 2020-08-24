#include <splashscreen.h>
#include <paintutils.h>
#include <appconstants.h>

#include <QPainter>
#include <QDate>

SplashScreen::SplashScreen(const QPixmap& pixmap, Qt::WindowFlags windowFlags)
    : QSplashScreen(pixmap, windowFlags)
{
}

void SplashScreen::drawContents(QPainter* painter)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(Qt::NoBrush);

    // Draw border frame
#if !defined(Q_OS_MACOS) // macOS draws a frame on every window by default
    painter->setPen(QColor(0, 0, 0, 80));
    painter->drawRect(rect());
#endif

    // Draw icon
    const QRectF iconRect(20, 20, 128, 128);
    const QPixmap& icon = PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), iconRect.size().toSize(), this);
    painter->drawPixmap(iconRect, icon, icon.rect());

    // Draw title
    QFont f(font());
    f.setPixelSize(f.pixelSize() + 7);
    f.setWeight(QFont::Light);
    const QString& title = QStringLiteral("%1 <a style=\"font-weight:normal;font-size:%3px\">v%2</a>")
            .arg(AppConstants::LABEL)
            .arg(QString(AppConstants::VERSION).left(3))
            .arg(f.pixelSize() - 7);
    const QRectF titleRect(iconRect.left() + 10, iconRect.bottom() + 24, 300, QFontMetrics(f).height() + 2);
    PaintUtils::drawHtml(painter, title, titleRect.topLeft(), f, palette().highlight().color(),
                         Qt::AlignLeft | Qt::AlignVCenter);

    // Draw subtitle
    f.setPixelSize(f.pixelSize() - 7);
    f.setWeight(QFont::Normal);
    const QString& subtitle = tr("Cross-Platform Application Development");
    const QRectF subtitleRect(titleRect.left(), titleRect.bottom() + 5,
                              QFontMetrics(f).horizontalAdvance(subtitle) + 2, QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->setPen(palette().text().color());
    painter->drawText(subtitleRect, subtitle, Qt::AlignLeft | Qt::AlignVCenter);

    // Draw informative text
    f.setPixelSize(f.pixelSize() - 2);
    const QString& informativeText = tr("Copyright © 2015 - %1 %2.\nAll Rights "
                                        "Reserved. For more details and\nlegal "
                                        "notices, please go to the About Screen.")
            .arg(QDate::currentDate().year()).arg(AppConstants::COMPANY_FULL);
    const QRectF informativeRect(subtitleRect.left(), subtitleRect.bottom() + 8,
                                 subtitleRect.width(), 3 * QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->setPen(palette().text().color().lighter(250));
    painter->drawText(informativeRect, informativeText, Qt::AlignLeft | Qt::AlignVCenter);

    // Draw message
    const QRectF messageRect(width() - QFontMetrics(f).horizontalAdvance(message()) - 21,
                             height() - QFontMetrics(f).height() - 22,
                             QFontMetrics(f).horizontalAdvance(message()) + 1,
                             QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->setPen(palette().text().color().lighter(200));
    painter->drawText(messageRect, message(), Qt::AlignRight | Qt::AlignVCenter);
}
