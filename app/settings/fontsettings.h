#ifndef FONTSETTINGS_H
#define FONTSETTINGS_H

#include <settings.h>

#include <QColor>
#include <QFont>

class FontSettings : public Settings
{
    Q_OBJECT

public:
    explicit FontSettings(QObject* parent = nullptr);
    explicit FontSettings(const QString& group, QObject* parent = nullptr);

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

#endif // FONTSETTINGS_H