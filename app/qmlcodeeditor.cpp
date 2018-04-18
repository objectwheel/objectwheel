#include <qmlcodeeditor.h>
#include <qmlformatter.h>

#include <QtWidgets>

#define COLOR_LINENUMBERAREA (QColor("#ececec"))
#define COLOR_CURRENTHIGHLIGHT (QColor("#f0f0f0"))
#define COLOR_LINENUMBERTEXT (QColor("#bfbfbf"))
#define COLOR_EDITORBACKGROUND Qt::white
#define SPACE_LINENUMBERAREALEFT 20
#define SPACE_LINENUMBERAREARIGHT 5
#define INTERVAL_COMPLETIONTIMER (10000)
#define TAB_SPACE ("    ")

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

bool blockVisible(const QString& block)
{
    for (int i = 0; i < block.size(); i++)
        if (!block.at(i).isSpace())
            return true;
    return false;
}

int leftCount(QString block)
{
    if (block[0] == 8233) //NOTE: Weird bug fix
        block.remove(0, 1);

    int leftCount = 0;
    for (int i = 0; i < block.size(); i++)
        if (block.at(i).isSpace())
            leftCount++;
        else
            break;
    return leftCount;
}

bool leftBracket(const QString& block)
{
    for (int i = block.size(); i > 0; i--) {
        if (block.at(i - 1) == '{')
            return true;
        if (block.at(i - 1) == '}')
            return false;
    }
    return false;
}

bool rightBracket(const QString& block)
{
    for (int i = 0; i < block.size(); i++) {
        if (block.at(i) == '}')
            return true;
        if (block.at(i) == '{')
            return false;
    }
    return false;
}

void alignBlock(QTextCursor textCursor, bool nonvisible = false)
{
    if (textCursor.isNull())
        return;

    auto cursor = textCursor;
    cursor.select(QTextCursor::BlockUnderCursor);
    auto currentLine = cursor.selectedText();

    if (nonvisible || blockVisible(currentLine)) {
        bool succeed;
        QString selectedText;
        int currentLeftCount = leftCount(currentLine);

        do {
            cursor.clearSelection();
            succeed = cursor.movePosition(QTextCursor::PreviousBlock);
            cursor.select(QTextCursor::BlockUnderCursor);
            selectedText = cursor.selectedText();
        } while (succeed && !blockVisible(selectedText));

        if (!blockVisible(selectedText)) {
            cursor = textCursor;
            cursor.movePosition(QTextCursor::StartOfBlock);
            cursor.insertText(TAB_SPACE);
            return;
        }

        int spaceCount = 0;
        int prevLeftCount = leftCount(selectedText);

        if (rightBracket(currentLine)) {
            cursor = textCursor;
            do {
                cursor.clearSelection();
                succeed = cursor.movePosition(QTextCursor::PreviousBlock);
                cursor.select(QTextCursor::BlockUnderCursor);
                selectedText = cursor.selectedText();
            } while (succeed && !leftBracket(selectedText));

            if (!leftBracket(selectedText)) {
                cursor = textCursor;
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText(TAB_SPACE);
                return;
            }

            prevLeftCount = leftCount(selectedText);
            spaceCount = prevLeftCount - currentLeftCount;
        } else {
            if (leftBracket(selectedText))
                spaceCount = prevLeftCount + QString(TAB_SPACE).size() - currentLeftCount;
            else
                spaceCount = prevLeftCount - currentLeftCount;
        }

        cursor = textCursor;
        cursor.movePosition(QTextCursor::StartOfBlock);
        if (spaceCount > 0)
            for (int i = 0; i < spaceCount; i++)
                cursor.insertText(" ");
        else
            for (int i = 0; i < qAbs(spaceCount); i++)
                cursor.deleteChar();
    } else {
        cursor.clearSelection();
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.insertText(TAB_SPACE);
    }
}

QmlCodeEditor::QmlCodeEditor(QWidget* parent)
    : QPlainTextEdit(parent)
{
    QPalette p(palette());
    p.setColor(QPalette::Base, COLOR_EDITORBACKGROUND);
    setPalette(p);

    setAcceptDrops(false);
    setWordWrapMode(QTextOption::NoWrap);

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
    _completer.popup()->setIconSize(QSize(16, 16));
    _completer.setWidget(this);
    connect(&_completer, SIGNAL(activated(QString)), this, SLOT(insertCompletion(QString)));

    _completionHelper.moveToThread(&_completionThread);
    connect(&_completionHelper, SIGNAL(extractionReady(const ExtractionResult&)),
            SLOT(handleExtractionResult(const ExtractionResult&)));
    _completionThread.start();

    _completionTimer.setInterval(INTERVAL_COMPLETIONTIMER);
    connect(&_completionTimer, SIGNAL(timeout()), SLOT(updateCompletion()));
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

void QmlCodeEditor::addErrorLine(int line)
{
    _errorLines << line;
    update();
}

void QmlCodeEditor::clearErrorLines()
{
    _errorLines.clear();
    update();
}

void QmlCodeEditor::reset()
{
    // TODO:
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
    painter.setRenderHint(QPainter::Antialiasing);
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

            pen.setWidthF(1);
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
            if (_errorLines.contains(number.toInt())) {
                painter.fillRect(0, top, lineNumberArea->width() - SPACE_LINENUMBERAREARIGHT,
                                 fontMetrics().height(), "#D02929");

                font.setBold(true);
                painter.setFont(font);
                pen.setColor("white");
                painter.setPen(pen);
            }
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
        item->setIcon(QIcon(":/images/keyword.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    for (auto name : result.properties) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/images/property.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    for (auto name : result.functions) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/images/function.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    for (auto name : result.types) {
        QStandardItem* item = new QStandardItem(name);
        item->setIcon(QIcon(":/images/type.png"));
        _model.setItem(row, 0, item);
        row++;
    }

    _completer.setModel(&_model);
}

void QmlCodeEditor::updateCompletion()
{
    QMetaObject::invokeMethod(&_completionHelper, "extractCompletions",
                              Qt::QueuedConnection,
                              Q_ARG(QString, toPlainText()));
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

QSize QmlCodeEditor::sizeHint() const
{
    return QSize(480, 680);
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
    } else {
        if ((e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return) &&
            ((e->modifiers() & Qt::ShiftModifier) == Qt::ShiftModifier))
            e->setModifiers(e->modifiers()&Qt::MetaModifier&Qt::KeypadModifier);
    }

    if (e->key() == Qt::Key_Tab) {
        if (textCursor().hasSelection()) {
            auto selectedText = textCursor().selectedText();
            QString prefix, suffix;
            for (int i = 0; i < selectedText.size() && selectedText.at(i).isSpace(); i++)
                prefix.append(selectedText.at(i));
            for (int i = selectedText.size(); i > 0 && selectedText.at(i - 1).isSpace(); i--)
                suffix.prepend(selectedText.at(i - 1));
            QmlFormatter::format(selectedText);
            if (!selectedText.simplified().replace( " ", "" ).isEmpty()) {
                auto finalText = selectedText.trimmed();
                finalText = prefix + finalText + suffix;
                textCursor().insertText(finalText);
            } else {
                QTextBlock block = textCursor().block();
                do {
                    alignBlock(QTextCursor(block));
                    block = block.next();
                } while (block.isValid() && block.position() < textCursor().selectionEnd());

            }
        } else {
            alignBlock(textCursor());
        }

        return;
    }

    if (!textCursor().hasSelection() &&
        (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)) {

        auto cursor = textCursor();
        cursor.beginEditBlock();
        cursor.select(QTextCursor::BlockUnderCursor);
        auto firstLine = cursor.selectedText();
        cursor.clearSelection();

        cursor.insertBlock();
        alignBlock(cursor, true);

        if (leftBracket(firstLine)) {
            cursor.movePosition(QTextCursor::NextBlock);
            cursor.insertText("}\n");
            cursor.movePosition(QTextCursor::PreviousBlock);
            alignBlock(cursor);
        }
        cursor.movePosition(QTextCursor::PreviousCharacter);
        cursor.insertText("a");
        cursor.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.endEditBlock();

        return;
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

    if (!isShortcut && (hasModifier || e->text().isEmpty()|| completionPrefix.length() < 2
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
