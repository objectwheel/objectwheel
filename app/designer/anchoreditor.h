#ifndef ANCHOREDITOR_H
#define ANCHOREDITOR_H

#include <QWidget>
#include <anchorline.h>

class QBoxLayout;
class AnchorRow;
class QPushButton;
class Control;

class AnchorEditor final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AnchorEditor)

public:
    explicit AnchorEditor(QWidget* parent = nullptr);

    void activate(Control* source, Control* target);

signals:
    void anchored(Control* sourceControl, AnchorLine::Type sourceLineType, const AnchorLine& targetLine);

private:
    QBoxLayout* m_layout;
    AnchorRow* m_leftRow;
    AnchorRow* m_rightRow;
    AnchorRow* m_topRow;
    AnchorRow* m_bottomRow;
    AnchorRow* m_horizontalCenterRow;
    AnchorRow* m_verticalCenterRow;
    QPushButton* m_closeButton;
};

#endif // ANCHOREDITOR_H
