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
    painter->setPen(QColor(0, 0, 0, 60));
    painter->drawRect(rect());
#endif

    // Draw icon
    const QRectF iconRect(20, 20, 128, 128);
    const QPixmap& icon = PaintUtils::pixmap(QStringLiteral(":/images/icon.png"), iconRect.size().toSize(), this);
    painter->drawPixmap(iconRect, icon, icon.rect());

    // Draw title
    QFont f(font());
    f.setPixelSize(f.pixelSize() + 8);
    f.setWeight(QFont::Light);
    const QString& title = QStringLiteral("%1 v%2")
            .arg(AppConstants::LABEL)
            .arg(QString(AppConstants::VERSION).left(3));
    const QRectF titleRect(iconRect.left() + 5, iconRect.bottom() + 20, 250, QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->setPen(palette().highlight().color());
    painter->drawText(titleRect, title, Qt::AlignLeft | Qt::AlignVCenter);

    // Draw subtitle
    f.setPixelSize(f.pixelSize() - 8);
    f.setWeight(QFont::Normal);
    const QString& subtitle = tr("Cross-Platform Application Development");
    const QRectF subtitleRect(titleRect.left(), titleRect.bottom() + 4,
                              titleRect.width(), QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->setPen(palette().text().color());
    painter->drawText(subtitleRect, subtitle, Qt::AlignLeft | Qt::AlignVCenter);

    // Draw informative text
    f.setPixelSize(f.pixelSize() - 2);
    const QString& informativeText = tr("Copyright © 2015 - %1 %2. All Rights "
                                        "Reserved. For more details and legal "
                                        "notices, please visit the About Screen.")
            .arg(QDate::currentDate().year()).arg(AppConstants::COMPANY_FULL);
    const QRectF informativeRect(subtitleRect.left(), subtitleRect.bottom() + 8,
                                 subtitleRect.width(), 3 * QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->setPen(palette().text().color().lighter(250));
    painter->drawText(informativeRect, informativeText, Qt::AlignLeft | Qt::AlignVCenter);

    // Draw message
    f.setPixelSize(f.pixelSize() + 1);
    const QRectF messageRect(width() - QFontMetrics(f).horizontalAdvance(message()) - 21,
                             height() - QFontMetrics(f).height() - 22,
                             QFontMetrics(f).horizontalAdvance(message()) + 1,
                             QFontMetrics(f).height() + 2);
    painter->setFont(f);
    painter->drawText(messageRect, message(), Qt::AlignRight | Qt::AlignVCenter);
}
