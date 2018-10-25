#include <linenumberband.h>
#include <rowbar.h>
#include <qmlcodedocument.h>
#include <qmlcodeeditor.h>
#include <codeeditorsettings.h>
#include <fontcolorssettings.h>

#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QTextCursor>

struct ExtraAreaPaintEventData
{
    ExtraAreaPaintEventData(const QmlCodeEditor *editor)
        : doc(editor->document())
        , documentLayout(qobject_cast<QPlainTextDocumentLayout*>(doc->documentLayout()))
        , selectionStart(editor->textCursor().selectionStart())
        , selectionEnd(editor->textCursor().selectionEnd())
        , fontMetrics(editor->rowBar()->font())
        , lineSpacing(fontMetrics.lineSpacing())
        , markWidth(/*m_marksVisible ? */lineSpacing/* : 0*/)
        , collapseColumnWidth(/*m_codeFoldingVisible ? foldBoxWidth(fontMetrics) :*/ 0)
        , extraAreaWidth(editor->rowBar()->width() - collapseColumnWidth)
        , currentLineNumberFormat(CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_CURRENT_LINE_NUMBER))
        , palette(editor->rowBar()->palette())
    {
        palette.setCurrentColorGroup(QPalette::Active);
    }
    QTextBlock block;
    const QTextDocument *doc;
    const QPlainTextDocumentLayout *documentLayout;
    const int selectionStart;
    const int selectionEnd;
    const QFontMetrics fontMetrics;
    const int lineSpacing;
    const int markWidth;
    const int collapseColumnWidth;
    const int extraAreaWidth;
    const QTextCharFormat currentLineNumberFormat;
    QPalette palette;
};

LineNumberBand::LineNumberBand(QmlCodeEditor* editor, QWidget* parent) : QWidget(parent)
  , m_qmlCodeEditor(editor)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

int LineNumberBand::calculatedWidth() const
{
    int digits = 1;
    int max = qMax(1, m_qmlCodeEditor->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 1 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

QSize LineNumberBand::sizeHint() const
{
    return QSize(calculatedWidth(), 0);
}

void LineNumberBand::paintEvent(QPaintEvent* e)
{
    ExtraAreaPaintEventData data(m_qmlCodeEditor);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    auto ce = m_qmlCodeEditor;
    auto block = ce->firstVisibleBlock();
    auto blockNumber = block.blockNumber();
    auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
    auto bottom = top + ce->blockBoundingRect(block).height();

    while (block.isValid() && top <= e->rect().bottom()) {
        auto f = font();
        if (ce->textCursor().block() == block)
            f.setBold(true);
        painter.setFont(f);

        if (block.isVisible() && bottom >= e->rect().top()) {
            auto number = QString::number(blockNumber + 1);
            painter.setPen(data.palette.color(QPalette::Dark));

            const bool selected = (
                        (data.selectionStart < data.block.position() + data.block.length()
                         && data.selectionEnd > data.block.position())
                        || (data.selectionStart == data.selectionEnd && data.selectionEnd == data.block.position())
                        );
            if (selected) {
                painter.save();
                QFont f = painter.font();
                f.setBold(data.currentLineNumberFormat.font().bold());
                f.setItalic(data.currentLineNumberFormat.font().italic());
                painter.setFont(f);
                painter.setPen(data.currentLineNumberFormat.foreground().color());
                if (data.currentLineNumberFormat.background() != Qt::NoBrush) {
                    painter.fillRect(QRectF(0, top,
                                            data.extraAreaWidth, bottom - top),
                                     data.currentLineNumberFormat.background().color());
                }
                painter.restore();
            }

            painter.drawText(0, top, width(), fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignTop, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
        ++blockNumber;
    }
}
