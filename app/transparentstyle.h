#ifndef TRANSPARENTSTYLE_H
#define TRANSPARENTSTYLE_H

#include <QProxyStyle>

class TransparentStyle : public QCommonStyle
{
public:
    explicit TransparentStyle(QObject* parent = nullptr);

    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget = nullptr) const override;
};

#endif // TRANSPARENTSTYLE_H