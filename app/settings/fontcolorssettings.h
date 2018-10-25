#ifndef FONTCOLORSSETTINGS_H
#define FONTCOLORSSETTINGS_H

#include <settings.h>

class FontColorsSettings : public Settings
{
    Q_OBJECT

public:
    explicit FontColorsSettings(QObject* parent = nullptr);
    explicit FontColorsSettings(const QString& group, QObject* parent = nullptr);

    void read() override;
    void write() override;
    void reset() override;

    int fontPixelSize;
    QString fontFamily;
    bool fontPreferThick;
    bool fontPreferAntialiasing;
};

#endif // FONTCOLORSSETTINGS_H