#include <breakpointband.h>
#include <blockdata.h>
#include <qmlcodedocument.h>
#include <qmlcodeeditor.h>

#include <QPainter>
#include <QPaintEvent>

BreakpointBand::BreakpointBand(QmlCodeEditor* editor, QWidget* parent) : QWidget(parent)
  , m_qmlCodeEditor(editor)
{
    setCursor(Qt::PointingHandCursor);
}

int BreakpointBand::calculatedWidth() const
{
    return 20;
}

QSize BreakpointBand::sizeHint() const
{
    return QSize(calculatedWidth(), 0);
}

void BreakpointBand::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto ce = m_qmlCodeEditor;
    auto block = ce->firstVisibleBlock();
    auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
    auto bottom = top + ce->blockBoundingRect(block).height();
    auto hg = qMin(fontMetrics().height() - 4, 10);

    while (block.isValid() && top <= e->rect().bottom()) {
        auto blockData = QmlCodeDocument::userData(block);

        if (blockData->hasBreakpointSet
                && block.isVisible() && bottom >= e->rect().top()) {
            painter.setPen("#98141F");
            painter.setBrush(QColor("#D41C18"));
            painter.drawEllipse(width() / 2.0 - hg / 2.0,
                                top + fontMetrics().height() / 2.0 - hg / 2.0, hg, hg);
        }

        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
    }
}

void BreakpointBand::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        auto pos = e->pos();
        auto ce = m_qmlCodeEditor;
        auto block = ce->firstVisibleBlock();
        auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
        auto bottom = top + ce->blockBoundingRect(block).height();

        while (block.isValid() && top <= rect().bottom()) {
            auto blockData = QmlCodeDocument::userData(block);

            if (pos.y() >= top && pos.y() <= bottom) {
                blockData->hasBreakpointSet = !blockData->hasBreakpointSet;
                update();
                break;
            }

            block = block.next();
            top = bottom;
            bottom = top + ce->blockBoundingRect(block).height();
        }
    }
    else
        QWidget::mouseReleaseEvent(e);
}