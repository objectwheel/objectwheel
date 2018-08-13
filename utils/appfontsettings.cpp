#include <appfontsettings.h>
#include <filemanager.h>

#include <QFontDatabase>
#include <QApplication>

void AppFontSettings::apply()
{
    for (const QString& fontName : lsfile(":/fonts"))
        QFontDatabase::addApplicationFont(":/fonts/" + fontName);

#if defined(Q_OS_MACOS)
    QFont font(".SF NS Display");
#elif defined(Q_OS_WIN)
    QFont font("Segoe UI");
#else
    QFont font("Open Sans");
#endif
    font.setPixelSize(defaultPixelSize());
    font.setStyleStrategy(QFont::PreferAntialias);

    QApplication::setFont(font);
}