#include <buttonflasher.h>
#include <QAbstractButton>
#include <QPainter>
#include <QtMath>

ButtonFlasher::ButtonFlasher(QAbstractButton* parent) : ApplicationStyle()
  , m_button(parent)
{
    setParent(parent);
    parent->setStyle(this);

    m_colorOpacity.setStartValue(0.0);
    m_colorOpacity.setEndValue(M_PI);
    m_colorOpacity.setEasingCurve(QEasingCurve::InOutCirc);
    connect(&m_colorOpacity, &QVariantAnimation::valueChanged, parent, qOverload<>(&QWidget::update));
}

void ButtonFlasher::flash(int timeout, int repeat)
{
    m_colorOpacity.setLoopCount(repeat);
    m_colorOpacity.setDuration(timeout * 2);
    if (m_colorOpacity.state() != QAbstractAnimation::Running)
        m_colorOpacity.start();
}

void ButtonFlasher::drawControl(QStyle::ControlElement element, const QStyleOption* option,
                                QPainter* painter, const QWidget* widget) const
{
    ApplicationStyle::drawControl(element, option, painter, widget);

    if (element == CE_PushButtonLabel) {
        painter->save();

        static const QColor highlight = "#157efb";
        const QRectF rect = m_button->rect();
        const qreal opacity = qSin(m_colorOpacity.currentValue().toReal());

        QPainterPath outlinePath;
        outlinePath.addRoundedRect(rect.adjusted(0.0, 0.5, 0.0, -0.5), 4.0, 4.0);
        outlinePath.addRoundedRect(rect.adjusted(0.5, 1.0, -0.5, -1.0), 3.5, 3.5);
        painter->setPen(Qt::NoPen);
        painter->setBrush(highlight);
        painter->setOpacity(opacity * 0.8);
        painter->drawPath(outlinePath);

        QPainterPath inlinePath;
        inlinePath.addRoundedRect(rect.adjusted(0.5, 1.0, -0.5, -1.0), 3.5, 3.5);
        inlinePath.addRoundedRect(rect.adjusted(1.5, 2.0, -1.5, -2.0), 3.0, 3.0);
        painter->setOpacity(opacity * 0.6);
        painter->fillPath(inlinePath, highlight);

        painter->restore();
    }
}
