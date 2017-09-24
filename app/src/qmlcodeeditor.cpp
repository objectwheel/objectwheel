#include <qmlcodeeditor.h>
#include <fit.h>
#include <QtWidgets>

#define COLOR_LINENUMBERAREA (QColor("#E3E7EA"))
#define COLOR_CURRENTHIGHLIGHT (QColor("#D7E5F2"))
#define COLOR_LINENUMBERTEXT (QColor("#929699"))
#define COLOR_EDITORBACKGROUND (QColor("#F3F7FA"))
#define SPACE_LINENUMBERAREALEFT fit(20)
#define SPACE_LINENUMBERAREARIGHT fit(5)
#define INTERVAL_COMPLETIONTIMER (5000)

using namespace Fit;

class LineNumberArea : public QWidget
{
    public:
        LineNumberArea(QmlCodeEditor* editor) : QWidget(editor) {
            codeEditor = editor;
        }

        QSize sizeHint() const override {
            return QSize(codeEditor->lineNumberAreaWidth(), 0);
        }

    protected:
        void paintEvent(QPaintEvent* event) override {
            codeEditor->lineNumberAreaPaintEvent(event);
        }

    private:
        QmlCodeEditor* codeEditor;
};

QmlCodeEditor::QmlCodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
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

    qRegisterMetaType<ExtractionResult>("ExtractionResult");
    _completer.setCompletionMode(QCompleter::PopupCompletion);
    _completer.setModelSorting(QCompleter::UnsortedModel);
    _completer.setCaseSensitivity(Qt::CaseInsensitive);
    _completer.setWrapAround(false);
    _completer.popup()->setIconSize(QSize(fit(16), fit(16)));
    _completer.setWidget(this);
    connect(&_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

    _completionHelper.moveToThread(&_completionThread);
    connect(&_completionHelper, SIGNAL(extractionReady(const ExtractionResult&)),
            SLOT(handleExtractionResult(const ExtractionResult&)));
    _completionThread.start();

    _completionTimer.setInterval(INTERVAL_COMPLETIONTIMER);
    connect(&_completionTimer, &QTimer::timeout, [=]{
        QMetaObject::invokeMethod(&_completionHelper, "extractCompletions",
                                  Qt::QueuedConnection,
                                  Q_ARG(QString, toPlainText()));
    });
    _completionTimer.start();
}

QmlCodeEditor::~QmlCodeEditor()
{
    _completionThread.quit();
    _completionThread.wait();
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

void QmlCodeEditor::resizeEvent(QResizeEvent* e)
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

void QmlCodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event)
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

void QmlCodeEditor::insertCompletion(const QString& completion)
{
    if (_completer.widget() != this)
        return;
    QTextCursor tc = textCursor();
    int extra = completion.length() - _completer.completionPrefix().length();
    tc.movePosition(QTextCursor::Left);
    tc.movePosition(QTextCursor::EndOfWord);
    tc.insertText(completion.right(extra));
    setTextCursor(tc);
}

void QmlCodeEditor::handleExtractionResult(const ExtractionResult& result)
{
    if (_completer.popup()->isVisible())
        return;

    _model.clear();
    _model.setRowCount(result.keywords.size() + result.properties.size() +
                       result.functions.size() + result.types.size());
    _model.setColumnCount(1);

    int row = 0;
    for (auto name : result.keywords) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/resources/images/keyword.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    for (auto name : result.properties) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/resources/images/property.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    for (auto name : result.functions) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/resources/images/function.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    for (auto name : result.types) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/resources/images/type.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    _completer.setModel(&_model);
}

QString QmlCodeEditor::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}

void QmlCodeEditor::focusInEvent(QFocusEvent* e)
{
    _completer.setWidget(this);
    QPlainTextEdit::focusInEvent(e);
}

void QmlCodeEditor::keyPressEvent(QKeyEvent* e)
{
    if (_completer.popup()->isVisible()) {
        // The following keys are forwarded by the completer to the widget
        switch (e->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                e->ignore();
                return; // let the completer do default behavior
            default:
                break;
        }
    }

    bool isShortcut = ((e->modifiers() & Qt::AltModifier) && e->key() == Qt::Key_Space); // ALT + Space
    if (!isShortcut) // do not process the shortcut when we have a completer
        QPlainTextEdit::keyPressEvent(e);

    const bool ctrlOrShift = e->modifiers() & (Qt::AltModifier | Qt::ShiftModifier);
    if ((ctrlOrShift && e->text().isEmpty()))
        return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (e->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 1
                        || eow.contains(e->text().right(1)))) {
        _completer.popup()->hide();
        return;
    }

    if (completionPrefix != _completer.completionPrefix()) {
        _completer.setCompletionPrefix(completionPrefix);
        _completer.popup()->setCurrentIndex(_completer.completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(_completer.popup()->sizeHintForColumn(0)
                + _completer.popup()->verticalScrollBar()->sizeHint().width());
    _completer.complete(cr); // popup it up!
}
