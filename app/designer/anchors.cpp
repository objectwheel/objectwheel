#include <anchors.h>

Anchors::Anchors(DesignerItem* item) : QObject(item)
  , m_margins(0)
  , m_topMargin(0)
  , m_bottomMargin(0)
  , m_leftMargin(0)
  , m_rightMargin(0)
  , m_verticalCenterOffset(0)
  , m_horizontalCenterOffset(0)
  , m_baselineOffset(0)
  , m_alignWhenCentered(true)
{
}

void Anchors::clear()
{
    m_top.invalidate();
    m_bottom.invalidate();
    m_left.invalidate();
    m_right.invalidate();
    m_verticalCenter.invalidate();
    m_horizontalCenter.invalidate();
    m_baseline.invalidate();
    m_fill.clear();
    m_centerIn.clear();
    m_margins = 0;
    m_topMargin = 0;
    m_bottomMargin = 0;
    m_leftMargin = 0;
    m_rightMargin = 0;
    m_verticalCenterOffset = 0;
    m_horizontalCenterOffset = 0;
    m_baselineOffset = 0;
    m_alignWhenCentered = true;
}

AnchorLine Anchors::top() const
{
    return m_top;
}

void Anchors::setTop(const AnchorLine& edge)
{
    m_top = edge;
}

AnchorLine Anchors::bottom() const
{
    return m_bottom;
}

void Anchors::setBottom(const AnchorLine& edge)
{
    m_bottom = edge;
}

AnchorLine Anchors::left() const
{
    return m_left;
}

void Anchors::setLeft(const AnchorLine& edge)
{
    m_left = edge;
}

AnchorLine Anchors::right() const
{
    return m_right;
}

void Anchors::setRight(const AnchorLine& edge)
{
    m_right = edge;
}

AnchorLine Anchors::verticalCenter() const
{
    return m_verticalCenter;
}

void Anchors::setVerticalCenter(const AnchorLine& edge)
{
    m_verticalCenter = edge;
}

AnchorLine Anchors::horizontalCenter() const
{
    return m_horizontalCenter;
}

void Anchors::setHorizontalCenter(const AnchorLine& edge)
{
    m_horizontalCenter = edge;
}

AnchorLine Anchors::baseline() const
{
    return m_baseline;
}

void Anchors::setBaseline(const AnchorLine& edge)
{
    m_baseline = edge;
}

DesignerItem* Anchors::fill() const
{
    return m_fill;
}

void Anchors::setFill(DesignerItem* fill)
{
    m_fill = fill;
}

DesignerItem* Anchors::centerIn() const
{
    return m_centerIn;
}

void Anchors::setCenterIn(DesignerItem* centerIn)
{
    m_centerIn = centerIn;
}

qreal Anchors::margins() const
{
    return m_margins;
}

void Anchors::setMargins(qreal margins)
{
    m_margins = margins;
}

qreal Anchors::topMargin() const
{
    return m_topMargin;
}

void Anchors::setTopMargin(qreal topMargin)
{
    m_topMargin = topMargin;
}

qreal Anchors::bottomMargin() const
{
    return m_bottomMargin;
}

void Anchors::setBottomMargin(qreal bottomMargin)
{
    m_bottomMargin = bottomMargin;
}

qreal Anchors::leftMargin() const
{
    return m_leftMargin;
}

void Anchors::setLeftMargin(qreal leftMargin)
{
    m_leftMargin = leftMargin;
}

qreal Anchors::rightMargin() const
{
    return m_rightMargin;
}

void Anchors::setRightMargin(qreal rightMargin)
{
    m_rightMargin = rightMargin;
}

qreal Anchors::verticalCenterOffset() const
{
    return m_verticalCenterOffset;
}

void Anchors::setVerticalCenterOffset(qreal verticalCenterOffset)
{
    m_verticalCenterOffset = verticalCenterOffset;
}

qreal Anchors::horizontalCenterOffset() const
{
    return m_horizontalCenterOffset;
}

void Anchors::setHorizontalCenterOffset(qreal horizontalCenterOffset)
{
    m_horizontalCenterOffset = horizontalCenterOffset;
}

qreal Anchors::baselineOffset() const
{
    return m_baselineOffset;
}

void Anchors::setBaselineOffset(qreal baselineOffset)
{
    m_baselineOffset = baselineOffset;
}

bool Anchors::alignWhenCentered() const
{
    return m_alignWhenCentered;
}

void Anchors::setAlignWhenCentered(bool alignWhenCentered)
{
    m_alignWhenCentered = alignWhenCentered;
}

AnchorLine::Types Anchors::usedAnchors() const
{
    AnchorLine::Types types = AnchorLine::NoAnchor;
    if (m_left.isValid())
        types |= AnchorLine::Left;
    if (m_right.isValid())
        types |= AnchorLine::Right;
    if (m_top.isValid())
        types |= AnchorLine::Top;
    if (m_bottom.isValid())
        types |= AnchorLine::Bottom;
    if (m_horizontalCenter.isValid())
        types |= AnchorLine::HorizontalCenter;
    if (m_verticalCenter.isValid())
        types |= AnchorLine::VerticalCenter;
    if (m_baseline.isValid())
        types |= AnchorLine::Baseline;
    if (m_fill)
        types |= AnchorLine::Fill;
    if (m_centerIn)
        types |= AnchorLine::Center;
    return types;
}
