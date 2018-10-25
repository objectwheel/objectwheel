#include <bracketband.h>
#include <blockdata.h>
#include <qmlcodedocument.h>
#include <qmlcodeeditor.h>
#include <textutils.h>
#include <utilityfunctions.h>

#include <QMouseEvent>
#include <QPainter>
#include <QStack>
#include <QTextDocument>
#include <QTimer>

BracketBand::BracketBand(QmlCodeEditor* editor, QWidget* parent) : QWidget(parent)
  , m_qmlCodeEditor(editor)
{
    setMouseTracking(true);
    connect(m_qmlCodeEditor, &QmlCodeEditor::updateRequest,
            this, &BracketBand::updateData, Qt::QueuedConnection);
}

int BracketBand::calculatedWidth() const
{
    return 20;
}

bool BracketBand::toggleFold(const QPoint& pos) const
{
    auto ce = m_qmlCodeEditor;
    auto startBlock = QTextBlock();
    auto block = ce->firstVisibleBlock();
    auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
    auto bottom = top + ce->blockBoundingRect(block).height();

    while (block.isValid() && top <= rect().bottom()) {
        auto blockData = QmlCodeDocument::userData(block);

        if (pos.y() >= top && pos.y() <= bottom) {
            if (blockData->state == BlockData::StartOn
                    || blockData->state == BlockData::StartOff) {
                if (blockData->state == BlockData::StartOn)
                    blockData->state = BlockData::StartOff;
                else
                    blockData->state = BlockData::StartOn;

                startBlock = block;
                break;
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
    }

    if (startBlock.isValid()) {
        QTextDocument document(m_qmlCodeEditor->toPlainText());
        UtilityFunctions::trimCommentsAndStrings(&document);

        auto lb = QLatin1Char('{');
        auto rb = QLatin1Char('}');

        block = startBlock;
        auto blockData = QmlCodeDocument::userData(block);
        auto blockText = document.findBlock(block.position()).text();

        QStack<int> bracketStack;
        if (blockData->state == BlockData::StartOff) {
            block = block.next();
            bracketStack.push(1);

            while (block.isValid()) {
                blockData = QmlCodeDocument::userData(block);
                blockText = document.findBlock(block.position()).text();

                for (int i = blockText.count(lb); i--;)
                    bracketStack.push(1);

                for (int i = blockText.count(rb); i--;) {
                    if (!bracketStack.isEmpty())
                        bracketStack.pop();
                }

                if (bracketStack.isEmpty())
                    break;

                block.setVisible(false);
                block = block.next();
            }
        } else {
            block = block.next();
            bracketStack.push(1);

            int level = 1;

            while (block.isValid()) {
                blockData = QmlCodeDocument::userData(block);
                blockText = document.findBlock(block.position()).text();

                for (int i = blockText.count(rb); i--;) {
                    if (!bracketStack.isEmpty())
                        bracketStack.pop();
                }

                if (blockData->state != BlockData::StartOn) {
                    if (bracketStack.size() < level)
                        --level;
                }

                if (level == bracketStack.size())
                    block.setVisible(true);

                if (blockData->state == BlockData::StartOn) {
                    if (bracketStack.size() == level)
                        ++level;
                }

                for (int i = blockText.count(lb); i--;)
                    bracketStack.push(1);

                if (bracketStack.isEmpty())
                    break;

                block = block.next();
            }
        }

        m_qmlCodeEditor->document()->documentLayout()->update();
        m_qmlCodeEditor->document()->adjustSize();

        return true;
    } else {
        return false;
    }
}

QSize BracketBand::sizeHint() const
{
    return QSize(calculatedWidth(), 0);
}

void BracketBand::paintEvent(QPaintEvent* e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.fillRect(e->rect(), m_qmlCodeEditor->viewport()->palette().base().color());

    auto ce = m_qmlCodeEditor;
    auto block = ce->firstVisibleBlock();
    auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
    auto bottom = top + ce->blockBoundingRect(block).height();
    QColor color /* WARNING = m_qmlCodeEditor->codeDocument()->fontSettings().toTextCharFormat(
                TextEditor::C_CURRENT_LINE).background().color()*/;
    color.setAlpha(128);
    qreal lineHeight = block.layout()->lineForTextPosition(0).rect().height();

    while (block.isValid() && top <= e->rect().bottom()) {
        auto blockData = QmlCodeDocument::userData(block);
        auto w = qMin(fontMetrics().height() - 4, 12); // Handle width
        auto hr = QRectF(width() - w - 0.5, top + fontMetrics().height() / 2.0 - w / 2.0, w, w);

        painter.setPen("#A5A5A5");

        if (block.isVisible() && bottom >= e->rect().top()) {
            if (m_qmlCodeEditor->textCursor().block() == block)
                painter.fillRect(QRectF(0, top, width(), lineHeight), color);

            if (blockData->state == BlockData::StartOn || blockData->state == BlockData::StartOff) {
                painter.drawRect(hr);
                painter.setPen("#555555");
                painter.drawLine(QPointF(hr.x() + 2.5, hr.center().y()), QPointF(hr.right() - 2.5, hr.center().y()));

                if (blockData->state == BlockData::StartOff) {
                    painter.drawLine(QPointF(hr.center().x(), hr.top() + 2.5),
                                     QPointF(hr.center().x(), hr.bottom() - 2.5));
                }
            } else if (blockData->state == BlockData::Line) {
                painter.drawLine(QPointF(hr.center().x(), top), QPointF(hr.center().x(), bottom));
            } else if (blockData->state == BlockData::MiddleEnd) {
                painter.drawLine(QPointF(hr.center().x(), top), QPointF(hr.center().x(), bottom));
                painter.drawLine(QPointF(hr.center().x(), hr.center().y()), QPointF(hr.right(), hr.center().y()));
            } else if (blockData->state == BlockData::End) {
                painter.drawLine(QPointF(hr.center().x(), top), QPointF(hr.center().x(), bottom));
                painter.drawLine(QPointF(hr.center().x(), bottom), QPointF(hr.right(), bottom));
            }
        }

        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
    }
}

void BracketBand::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        if (!toggleFold(e->pos()))
            QWidget::mouseReleaseEvent(e);
    }
}

void BracketBand::updateData()
{
    QTextDocument document(m_qmlCodeEditor->toPlainText());
    UtilityFunctions::trimCommentsAndStrings(&document);

    auto ce = m_qmlCodeEditor;
    auto block = ce->document()->firstBlock();

    QStack<int> bracketStack;
    while (block.isValid()) {
        auto lb = QLatin1Char('{');
        auto rb = QLatin1Char('}');
        auto blockData = QmlCodeDocument::userData(block);
        auto blockText = document.findBlock(block.position()).text();

        /* Invisible lines (which means collapsed ones) ignored
           that means we leave them with their previous state */
        if (!block.isVisible()) {
            block = block.next();
            continue;
        }

        blockData->state = BlockData::NoBlock;

        /* Find out if current line is a block start */
        if (blockText.count(lb) > blockText.count(rb)) {
            blockData->state = BlockData::StartOn;
        } else if (blockText.count(lb) > 0 && blockText.count(lb) == blockText.count(rb)) {
            for (int i = blockText.size() - 1; i >= 0; i--) {
                if (blockText.at(i) == lb) {
                    blockData->state = BlockData::StartOn;
                    break;
                }
            }
        }

        if (blockData->state == BlockData::StartOn && !block.next().isVisible())
            blockData->state = BlockData::StartOff;

        /* Find out other block states */
        for (int i = blockText.count(lb); i--;)
            bracketStack.push(1);

        for (int i = blockText.count(rb); i--;) {
            if (!bracketStack.isEmpty())
                bracketStack.pop();
        }

        if (bracketStack.isEmpty()) {
            auto previousBlockData = QmlCodeDocument::userData(block.previous());
            if (previousBlockData) {
                if (previousBlockData->state == BlockData::Line)
                    blockData->state = BlockData::End;
                else if (previousBlockData->state == BlockData::MiddleEnd)
                    blockData->state = BlockData::End;
                else
                    blockData->state = BlockData::NoBlock;
            }
        } else if (blockData->state != BlockData::StartOn) {
            if (blockData->state != BlockData::StartOff) {
                if (blockText.count(rb) > 0)
                    blockData->state = BlockData::MiddleEnd;
                else
                    blockData->state = BlockData::Line;
            }
        }

        block = block.next();
    }

    update();
}
