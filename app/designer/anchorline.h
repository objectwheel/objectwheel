#ifndef ANCHORLINE_H
#define ANCHORLINE_H

#include <QtGlobal>

class Control;
class AnchorLine final
{
public:
    enum Type {
        Invalid = 0x0,
        NoAnchor =Invalid,
        Left = 0x01,
        Right = 0x02,
        Top = 0x04,
        Bottom = 0x08,
        HorizontalCenter = 0x10,
        VerticalCenter = 0x20,
        Baseline = 0x40,

        Fill =  Left | Right | Top | Bottom,
        Center = VerticalCenter | HorizontalCenter,
        VerticalMask = Left | Right | HorizontalCenter,
        HorizontalMask = Top | Bottom | VerticalCenter | Baseline,
        AllMask = VerticalMask | HorizontalMask
    };

public:
    AnchorLine();
    AnchorLine(Control* control, Type type);
    Type type() const;
    qreal margin() const;
    bool isValid() const;
    Control* control() const;

    static bool isVertical(Type type);
    static bool isHorizontal(Type type);

private:
    Control* m_control;
    Type m_type;
};

#endif // ANCHORLINE_H