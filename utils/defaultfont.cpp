#include <defaultfont.h>
#include <filemanager.h>
#include <QFontDatabase>

void DefaultFont::load()
{
    static const char* fontPath = ":/fonts";
    for (const QString& fontName : lsfile(fontPath))
        QFontDatabase::addApplicationFont(fontPath + fontName);
}

QFont DefaultFont::font()
{ 
#if defined(Q_OS_MACOS)
    QFont font(".SF NS Display");
#elif defined(Q_OS_WIN)
    QFont font("Segoe UI");
#else
    QFont font("Roboto");
#endif
    font.setPixelSize(13);
    font.setStyleStrategy(QFont::PreferAntialias);
    return font;
}