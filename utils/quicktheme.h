#ifndef QUICKTHEME_H
#define QUICKTHEME_H

#include <QtGlobal>

class QuickTheme final {
    Q_DISABLE_COPY(QuickTheme)

public:
    static void setTheme(const QString& projectDir, int* version = nullptr);

private:
    QuickTheme() {}
};

#endif // QUICKTHEME_H