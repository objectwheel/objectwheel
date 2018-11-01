#ifndef TRANSPARENTSTYLE_H
#define TRANSPARENTSTYLE_H

#include <applicationstyle.h>

class TransparentStyle : public ApplicationStyle
{
    Q_OBJECT
    Q_DISABLE_COPY(TransparentStyle)

public:
    explicit TransparentStyle(QObject* parent = nullptr);

    static void attach(QWidget* widget);

    QRect subElementRect(QStyle::SubElement element, const QStyleOption *option,
                         const QWidget *widget = nullptr) const override;
    int styleHint(QStyle::StyleHint hint, const QStyleOption *option = nullptr,
                  const QWidget *widget = nullptr, QStyleHintReturn *returnData = nullptr) const override;
    int pixelMetric(QStyle::PixelMetric metric, const QStyleOption *option = nullptr,
                    const QWidget *widget = nullptr) const override;
    void drawPrimitive(QStyle::PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget = nullptr) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption *option, QPainter *painter,
                     const QWidget *widget = nullptr) const override;
    void drawComplexControl(QStyle::ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget = nullptr) const override;
    QStyle::SubControl hitTestComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                                             const QPoint& point, const QWidget* widget) const override;
    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption *option,
                           const QSize &contentsSize, const QWidget *widget = nullptr) const override;
    QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex *option,
                         QStyle::SubControl subControl, const QWidget *widget = nullptr) const override;
};

#endif // TRANSPARENTSTYLE_H