#include <anchorline.h>
#include <control.h>

AnchorLine::AnchorLine()
    : m_control(nullptr)
    , m_type(Invalid)
{}

AnchorLine::AnchorLine(Control* control, Type type)
    : m_control(control),
      m_type(type)
{}

AnchorLine::Type AnchorLine::type() const
{
    return m_type;
}

qreal AnchorLine::margin() const
{
    if (!isValid())
        return 0;

    qreal margin = 0;

    if (m_type == Top)
        margin = m_control->anchors().value("anchors.topMargin").toReal();
    else if (m_type == Bottom)
        margin = m_control->anchors().value("anchors.bottomMargin").toReal();
    else if (m_type == Left)
        margin = m_control->anchors().value("anchors.leftMargin").toReal();
    else if (m_type == Right)
        margin = m_control->anchors().value("anchors.rightMargin").toReal();

    if (margin == 0)
        return m_control->anchors().value("anchors.margins").toReal();

    return margin;
}

bool AnchorLine::isValid() const
{
    return m_type != Invalid && m_control;
}

bool AnchorLine::isHorizontal(Type type)
{
    return type & HorizontalMask;
}

bool AnchorLine::isVertical(Type type)
{
    return type & VerticalMask;
}

Control* AnchorLine::control() const
{
    return m_control;
}
