#ifndef ROWBAR_H
#define ROWBAR_H

#include <QWidget>

class BracketBand;
class BreakpointBand;
class LineNumberBand;
class MarkBand;
class QHBoxLayout;
class QmlCodeEditor;

class RowBar : public QWidget
{
    Q_OBJECT

public:
    explicit RowBar(QmlCodeEditor* editor, QWidget* parent = nullptr);
    int calculatedWidth() const;

    BracketBand* bracketBand() const;
    LineNumberBand* linenumberBand() const;
    MarkBand* markBand() const;
    BreakpointBand* breakpointBand() const;

private:
    QSize sizeHint() const override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e) override;

private:
    QmlCodeEditor* m_qmlCodeEditor;
    QHBoxLayout* m_layout;
    BreakpointBand* m_breakpointBand;
    MarkBand* m_markBand;
    LineNumberBand* m_linenumberBand;
    BracketBand* m_bracketBand;
};

#endif // ROWBAR_H