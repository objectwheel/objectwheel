#ifndef ANCHORLINE_H
#define ANCHORLINE_H

#include <QPointer>

class Control;
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
    AnchorLine(Type type, Control* control);

    Type type() const;
    void setType(Type type);

    Control* control() const;
    void setControl(Control* control);

    void invalidate();
    bool isValid() const;
    bool isVertical() const;
    bool isHorizontal() const;
    bool isOffset() const;
    bool isFillCenter() const;

    static bool isVertical(Type type);
    static bool isHorizontal(Type type);
    static bool isOffset(Type type);
    static bool isFillCenter(Type type);

private:
    Type m_type;
    QPointer<Control> m_control;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(AnchorLine::Types)

#endif // ANCHORLINE_H
