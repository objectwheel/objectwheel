#ifndef FONTCOLORSSETTINGS_H
#define FONTCOLORSSETTINGS_H

#include <settings.h>

#include <QColor>
#include <QFont>

class FontColorsSettings : public Settings
{
    Q_OBJECT

public:
    explicit FontColorsSettings(QObject* parent = nullptr);
    explicit FontColorsSettings(const QString& group, QObject* parent = nullptr);

    void read() override;
    void write() override;
    void reset() override;

    bool hdpiEnabled;
    bool bottomPanesPop;
    bool fontPreferThick;
    bool fontPreferAntialiasing;
    int fontPixelSize;
    QString fontFamily;
    QString theme;
    QString language;
    QColor topBarColor;
    QColor leftBarColor;
    QString visibleBottomPane;
};

#endif // FONTCOLORSSETTINGS_H