#ifndef APPLICATIONSTYLE_H
#define APPLICATIONSTYLE_H

#include <private/qfusionstyle_p.h>

class QFocusFrame;
class ApplicationStyle : public QFusionStyle
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationStyle)
    Q_DECLARE_PRIVATE(QFusionStyle)

public:
    enum ButtonStyle {
        Push, // Default for push buttons
        Disclosure,  // Default for anything else
        Help,
        TexturedRounded,
        Combo // Default for combo boxes
    };
    Q_ENUM(ButtonStyle)

    static ButtonStyle buttonStyle(const QWidget* widget);
    static void setButtonStyle(QWidget* widget, ButtonStyle buttonStyle);
    static bool highlightingDisabledForCheckedState(const QWidget* widget);
    static void setHighlightingDisabledForCheckedState(QWidget* widget, bool highlightDisabled);
    static QPointF visualPos(Qt::LayoutDirection direction, const QRectF& boundingRect,
                             const QPointF& logicalPos);
public:
    ApplicationStyle();
    ~ApplicationStyle() override;

    QSize sizeFromContents(QStyle::ContentsType type, const QStyleOption* option,
                           const QSize& contentsSize, const QWidget* widget) const override;
    QRect subControlRect(QStyle::ComplexControl control, const QStyleOptionComplex* option,
                         QStyle::SubControl subControl, const QWidget* widget) const override;
    QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* opt,
                           const QWidget* widget) const override;
    QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption* option,
                       const QWidget* widget) const override;
    int styleHint(StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr,
                  QStyleHintReturn* returnData = nullptr) const override;
    int pixelMetric(PixelMetric metric, const QStyleOption* option,
                    const QWidget* widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption* option, QPainter* painter,
                       const QWidget* widget) const override;
    void drawControl(QStyle::ControlElement element, const QStyleOption* option,
                     QPainter* painter, const QWidget* widget) const override;
    void drawComplexControl(ComplexControl control, const QStyleOptionComplex* option,
                            QPainter* painter, const QWidget* widget) const override;
    void drawItemText(QPainter* painter, const QRect& rect, int alignment, const QPalette& pal,
                      bool enabled, const QString& text, QPalette::ColorRole textRole) const override;
    void drawItemText(QPainter* painter, const QRectF& rect, int alignment, const QPalette& pal,
                      bool enabled, const QString& text, QPalette::ColorRole textRole) const;
    void polish(QWidget* w) override;
    void unpolish(QWidget* w) override;
    bool event(QEvent* event) override;

private:
    QFocusFrame* m_focusFrame;
};

#endif // APPLICATIONSTYLE_H