#ifndef APPLICATIONSTYLE_H
#define APPLICATIONSTYLE_H

#include <QProxyStyle>

class ApplicationStyle : public QProxyStyle
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationStyle)

public:
    ApplicationStyle();

    int styleHint(StyleHint hint, const QStyleOption *option, const QWidget *widget,
                  QStyleHintReturn *returnData) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption *option,
                    const QWidget *widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter,
                       const QWidget *widget) const override;

    void drawControl(QStyle::ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;


    void polish(QWidget* w) override;

    void unpolish(QWidget* w) override;

};

#endif // APPLICATIONSTYLE_H