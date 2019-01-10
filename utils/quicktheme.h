#ifndef QUICKTHEME_H
#define QUICKTHEME_H

class QString;
namespace QuickTheme
{
void setTheme(const QString& projectDir, int* version = nullptr);
}

#endif // QUICKTHEME_H