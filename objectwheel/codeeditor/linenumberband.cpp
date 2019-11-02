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
        , collapseColumnWidth(/*m_codeFoldingVisible ? foldBoxWidth(fontMetrics) :*/ 0)
        , extraAreaWidth(editor->rowBar()->width() - collapseColumnWidth)
        , lineNumberFormat(
              CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_LINE_NUMBER))
        , currentLineNumberFormat(
              CodeEditorSettings::fontColorsSettings()->toTextCharFormat(C_CURRENT_LINE_NUMBER))
    {
    }
    QTextBlock block;
    const QTextDocument *doc;
    const QPlainTextDocumentLayout *documentLayout;
    const int selectionStart;
    const int selectionEnd;
    const int collapseColumnWidth;
    const int extraAreaWidth;
    const QTextCharFormat lineNumberFormat;
    const QTextCharFormat currentLineNumberFormat;
};

LineNumberBand::LineNumberBand(QmlCodeEditor* editor, QWidget* parent) : QWidget(parent)
  , m_qmlCodeEditor(editor)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    connect(CodeEditorSettings::instance(), &CodeEditorSettings::fontColorsSettingsChanged,
            this, qOverload<>(&RowBar::update));
}

int LineNumberBand::calculatedWidth() const
{
    int digits = 1;
    int max = qMax(1, m_qmlCodeEditor->document()->blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

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
        if (ce->textCursor().block() == block)
            painter.setFont(data.currentLineNumberFormat.font());
        else
            painter.setFont(data.lineNumberFormat.font());

        if (ce->textCursor().block() == block)
            painter.setPen(data.currentLineNumberFormat.foreground().color());
        else
            painter.setPen(data.lineNumberFormat.foreground().color());

        if (block.isVisible() && bottom >= e->rect().top()) {
            const QString& number = QString::number(blockNumber + 1);

            const bool selected = block.position() + block.length() > data.selectionStart
                    && block.position() < data.selectionEnd;

            if (selected) {
                painter.setFont(data.currentLineNumberFormat.font());
                painter.setPen(data.currentLineNumberFormat.foreground().color());
            }

            painter.drawText(0, top, width() - 2, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignTop, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + ce->blockBoundingRect(block).height();
        ++blockNumber;
    }
}
