#ifndef ANCHORLINE_H
#define ANCHORLINE_H

#include <designeritem.h>
#include <QPointer>

class AnchorLine final
{
public:
    enum Type {
        Invalid = 0x0,
        NoAnchor = Invalid,
        Left = 0x01,
        Right = 0x02,
        Top = 0x04,
        Bottom = 0x08,
        HorizontalCenter = 0x10,
        VerticalCenter = 0x20,
        Baseline = 0x40,

        Fill = Left | Right | Top | Bottom,
        Center = VerticalCenter | HorizontalCenter,
        VerticalMask = Left | Right | HorizontalCenter,
        HorizontalMask = Top | Bottom | VerticalCenter | Baseline,
        AllMask = VerticalMask | HorizontalMask
    };
    Q_DECLARE_FLAGS(Types, Type)

public:
    AnchorLine();
    AnchorLine(Type type, DesignerItem* item);

    Type type() const;
    void setType(Type type);

    DesignerItem* item() const;
    void setItem(DesignerItem* item);

    void invalidate();
    bool isValid() const;
    bool isVertical() const;
    bool isHorizontal() const;

    static bool isVertical(Type type);
    static bool isHorizontal(Type type);

private:
    Type m_type;
    QPointer<DesignerItem> m_item;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AnchorLine::Types)

#endif // ANCHORLINE_H
