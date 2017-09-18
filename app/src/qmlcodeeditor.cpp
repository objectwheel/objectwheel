#include <qmlcodeeditor.h>
#include <fit.h>

#include <QtWidgets>

#define COLOR_LINENUMBERAREA (QColor("#E3E7EA"))
#define COLOR_CURRENTHIGHLIGHT (QColor("#D8EBFF"))
#define COLOR_LINENUMBERTEXT (QColor("#A0A4A7"))
#define COLOR_EDITORBACKGROUND (QColor("#F3F7FA"))
#define SPACE_LINENUMBERAREALEFT fit(25)
#define SPACE_LINENUMBERAREARIGHT fit(5)

using namespace Fit;

class LineNumberArea : public QWidget
{
    public:
        LineNumberArea(QmlCodeEditor *editor) : QWidget(editor) {
            codeEditor = editor;
        }

        QSize sizeHint() const override {
            return QSize(codeEditor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
            codeEditor->lineNumberAreaPaintEvent(event);
        }

    private:
        QmlCodeEditor *codeEditor;
};

QmlCodeEditor::QmlCodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , highlighter(document())
{
    QPalette p(palette());
    p.setColor(QPalette::Base, COLOR_EDITORBACKGROUND);
    setPalette(p);

    lineNumberArea = new LineNumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberArea(QRect,int)));
    connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int QmlCodeEditor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

    return space + SPACE_LINENUMBERAREALEFT;
}

void QmlCodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void QmlCodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void QmlCodeEditor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void QmlCodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = COLOR_CURRENTHIGHLIGHT;

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void QmlCodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), COLOR_LINENUMBERAREA);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int) blockBoundingRect(block).height();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QPen pen;
            QFont font;
            QString number = QString::number(blockNumber + 1);

            if (textCursor().hasSelection() &&
                top >= blockBoundingGeometry(document()->findBlock(textCursor().selectionStart())).translated(contentOffset()).top() &&
                bottom <= blockBoundingGeometry(document()->findBlock(textCursor().selectionEnd())).translated(contentOffset()).bottom()) {
                font.setBold(true);
                pen.setColor(COLOR_LINENUMBERTEXT.darker());
            } else if (!textCursor().hasSelection() && textCursor().blockNumber() == blockNumber) {
                font.setBold(true);
                pen.setColor(COLOR_LINENUMBERTEXT.darker());
            } else {
                font.setBold(false);
                pen.setColor(COLOR_LINENUMBERTEXT);
            }

            painter.setFont(font);
            painter.setPen(pen);
            painter.drawText(0, top, lineNumberArea->width() - SPACE_LINENUMBERAREARIGHT,
                             fontMetrics().height(), Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + (int) blockBoundingRect(block).height();
        ++blockNumber;
    }
}
