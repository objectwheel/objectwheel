#include <anchorline.h>
#include <control.h>

AnchorLine::AnchorLine() : m_type(Invalid)
{
}

AnchorLine::AnchorLine(AnchorLine::Type type, Control* control) : m_type(type)
  , m_control(control)
{
}

AnchorLine::Type AnchorLine::type() const
{
    return m_type;
}

void AnchorLine::setType(AnchorLine::Type type)
{
    m_type = type;
}

Control* AnchorLine::control() const
{
    return m_control;
}

void AnchorLine::setControl(Control* control)
{
    m_control = control;
}

void AnchorLine::invalidate()
{
    m_type = Invalid;
    m_control.clear();
}

bool AnchorLine::isValid() const
{
    return m_type != Invalid && m_control;
}

bool AnchorLine::isVertical() const
{
    return isVertical(m_type);
}

bool AnchorLine::isHorizontal() const
{
    return isHorizontal(m_type);
}

bool AnchorLine::isVertical(Type type)
{
    return type & VerticalMask;
}

bool AnchorLine::isHorizontal(Type type)
{
    return type & HorizontalMask;
}
