#include <anchorline.h>

AnchorLine::AnchorLine() : m_type(Invalid)
{
}

AnchorLine::AnchorLine(AnchorLine::Type type, DesignerItem* item)
    : m_type(type)
    , m_item(item)
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

DesignerItem* AnchorLine::item() const
{
    return m_item;
}

void AnchorLine::setItem(DesignerItem* item)
{
    m_item = item;
}

void AnchorLine::invalidate()
{
    m_type = Invalid;
    m_item.clear();
}

bool AnchorLine::isValid() const
{
    return m_type != Invalid && m_item;
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
