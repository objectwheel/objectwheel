#include <markband.h>
#include <qmlcodedocument.h>
#include <qmlcodeeditor.h>
#include <utils/tooltip/tooltip.h>

#include <QPainter>
#include <QPaintEvent>

using namespace Utils;

QPixmap pixmap(Mark::Type type)
{
//    switch (type) {
//    case Mark::Type::Note: {
//        const static auto pixmap = Utils::Icons::INFO.pixmap();
//        return pixmap;
//    }
//    case Mark::Type::Warning: {
//        const static auto pixmap = Utils::Icons::WARNING.pixmap();
//        return pixmap;
//    }
//    case Mark::Type::CodeModelWarning: {
//        const static auto pixmap = Utils::Icons::CODEMODEL_WARNING.pixmap();
//        return pixmap;
//    }
//    case Mark::Type::Bug: {
//        const static auto pixmap =
//                Icon({
//                         {":/utils/images/bugfill.png", Theme::BackgroundColorNormal},
//                         {":/utils/images/bug.png", Theme::IconsInterruptColor}
//                     }, Icon::Tint).pixmap();

//        return pixmap;
//    }
//    case Mark::Type::Todo: {
//        const static auto pixmap =
//                Icon({
//                         {":/utils/images/tasklist.png", Theme::IconsCodeModelFunctionColor}
//                     }, Icon::Tint).pixmap();
//        return pixmap;
//    }
//    case Mark::Type::FixMe: {
//        const static auto pixmap = Utils::Icons::PROJECT.pixmap();
//        return pixmap;
//    }
//    case Mark::Type::CodeModelError: {
//        const static auto pixmap = Utils::Icons::CODEMODEL_ERROR.pixmap();
//        return pixmap;
//    }
//    case Mark::Type::Error: {
//        const static auto pixmap = Utils::Icons::CRITICAL_TOOLBAR.pixmap();
//        return pixmap;
//    }
//    default: {
//        const static auto pixmap = Utils::Icons::EMPTY16.pixmap();
//        return pixmap;
//    }
//    }
    return {};
}

MarkBand::MarkBand(QmlCodeEditor* editor, QWidget* parent) : QWidget(parent)
  , m_qmlCodeEditor(editor)
{
    setMouseTracking(true);
}

int MarkBand::calculatedWidth() const
{
    return 20;
}

QSize MarkBand::sizeHint() const
{
    return QSize(calculatedWidth(), 0);
}

void MarkBand::leaveEvent(QEvent* e)
{
    ToolTip::hide();
    QWidget::leaveEvent(e);
}

void MarkBand::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto ce = m_qmlCodeEditor;
    auto block = ce->firstVisibleBlock();
    auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
    auto bottom = top + ce->blockBoundingRect(block).height();
    auto hg = fontMetrics().height();

    while (block.isValid() && top <= e->rect().bottom()) {
        if (BlockData* blockData = QmlCodeDocument::userData(block)) {
            if (blockData->mark.type != Mark::NoMark && block.isVisible() && bottom >= e->rect().top())
                painter.drawPixmap(width() / 2.0 - hg / 2.0, top, hg, hg, pixmap(blockData->mark.type));
        }
        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
    }
}

void MarkBand::mouseMoveEvent(QMouseEvent* e)
{
    auto pos = e->pos();
    auto ce = m_qmlCodeEditor;
    auto block = ce->firstVisibleBlock();
    auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
    auto bottom = top + ce->blockBoundingRect(block).height();

    while (block.isValid() && top <= rect().bottom()) {
        if (pos.y() >= top && pos.y() <= bottom) {
            if (BlockData* blockData = QmlCodeDocument::userData(block)) {
                if (blockData->mark.type == Mark::NoMark)
                    ToolTip::hide();
                else
                    ToolTip::show(mapToGlobal(pos), blockData->mark.message, this);
            }
            break;
        }

        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
    }

    QWidget::mouseMoveEvent(e);
}

void MarkBand::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        auto pos = e->pos();
        auto ce = m_qmlCodeEditor;
        auto block = ce->firstVisibleBlock();
        auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
        auto bottom = top + ce->blockBoundingRect(block).height();

        while (block.isValid() && top <= rect().bottom()) {
            if (pos.y() >= top && pos.y() <= bottom) {
                if (BlockData* blockData = QmlCodeDocument::userData(block)) {
                    if (blockData->mark.type != Mark::NoMark)
                        emit markActivated(blockData->mark);
                }
                break;
            }

            block = block.next();
            top = bottom;
            bottom = top + ce->blockBoundingRect(block).height();
        }
    }

    QWidget::mouseReleaseEvent(e);
}
