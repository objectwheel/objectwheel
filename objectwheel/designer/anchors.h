#ifndef ANCHORS_H
#define ANCHORS_H

#include <anchorline.h>

class Anchors final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Anchors)

public:
    explicit Anchors(Control* control = nullptr);

    AnchorLine top() const;
    void setTop(const AnchorLine& edge);

    AnchorLine bottom() const;
    void setBottom(const AnchorLine& edge);

    AnchorLine left() const;
    void setLeft(const AnchorLine& edge);

    AnchorLine right() const;
    void setRight(const AnchorLine& edge);

    AnchorLine verticalCenter() const;
    void setVerticalCenter(const AnchorLine& edge);

    AnchorLine horizontalCenter() const;
    void setHorizontalCenter(const AnchorLine& edge);

    AnchorLine baseline() const;
    void setBaseline(const AnchorLine& edge);

    Control* fill() const;
    void setFill(Control* fill);

    Control* centerIn() const;
    void setCenterIn(Control* centerIn);

    qreal margins() const;
    void setMargins(qreal);

    qreal topMargin() const;
    void setTopMargin(qreal);

    qreal bottomMargin() const;
    void setBottomMargin(qreal);

    qreal leftMargin() const;
    void setLeftMargin(qreal);

    qreal rightMargin() const;
    void setRightMargin(qreal);

    qreal verticalCenterOffset() const;
    void setVerticalCenterOffset(qreal);

    qreal horizontalCenterOffset() const;
    void setHorizontalCenterOffset(qreal);

    qreal baselineOffset() const;
    void setBaselineOffset(qreal);

    bool alignWhenCentered() const;
    void setAlignWhenCentered(bool);

private:
    AnchorLine m_top;
    AnchorLine m_bottom;
    AnchorLine m_left;
    AnchorLine m_right;
    AnchorLine m_verticalCenter;
    AnchorLine m_horizontalCenter;
    AnchorLine m_baseline;
    QPointer<Control> m_fill;
    QPointer<Control> m_centerIn;
    qreal m_margins;
    qreal m_topMargin;
    qreal m_bottomMargin;
    qreal m_leftMargin;
    qreal m_rightMargin;
    qreal m_verticalCenterOffset;
    qreal m_horizontalCenterOffset;
    qreal m_baselineOffset;
    bool m_alignWhenCentered;
};

#endif // ANCHORS_H
