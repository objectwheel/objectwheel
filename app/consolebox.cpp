#include <consolebox.h>
#include <interpreterbackend.h>
#include <css.h>
#include <outputpane.h>

#include <QTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>

ConsoleBox::ConsoleBox(OutputPane* outputPane) : QWidget(outputPane)
  , _outputPane(outputPane)
{
    _layout = new QVBoxLayout(this);
    _textEdit = new QTextEdit;

    _layout->setSpacing(0);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_textEdit);

    _textEdit->setReadOnly(true);
    _textEdit->setFocusPolicy(Qt::NoFocus);
    _textEdit->setWordWrapMode(QTextOption::WordWrap);
    _textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _textEdit->verticalScrollBar()->setStyleSheet(CSS::ScrollBar);

    QFont f;
    f.setPixelSize(f.pixelSize() - 1);
    _textEdit->setFont(f);

    QPalette p1(_textEdit->palette());
    p1.setColor(QPalette::Base, QColor("#F5F9FC"));
    p1.setColor(QPalette::Highlight, QColor("#d0d4d7"));
    p1.setColor(QPalette::Text, QColor("#202427"));
    _textEdit->setPalette(p1);

    connect(InterpreterBackend::instance(), SIGNAL(standardError(QString)),
      SLOT(onStandardError(QString)));
    connect(InterpreterBackend::instance(), SIGNAL(standardOutput(QString)),
            SLOT(onStandardOutput(QString)));
}

bool ConsoleBox::isClean() const
{
    return _textEdit->toPlainText().isEmpty();
}

void ConsoleBox::print(const QString& text)
{
    printFormatted(text, _textEdit->palette().text().color(), QFont::Normal);
}

void ConsoleBox::printError(const QString& text)
{
    printFormatted(text, "#AA0000", QFont::Normal);
}

void ConsoleBox::printFormatted(const QString& text, const QColor& color, QFont::Weight weight)
{
    auto scrollBar = _textEdit->verticalScrollBar();
    bool atEnd = scrollBar->value() > scrollBar->maximum() * 0.8;
    auto document = _textEdit->document();
    QTextCursor cursor(document);

    cursor.movePosition(QTextCursor::End);
    int pos = cursor.position();

    cursor.insertText(text);
    cursor.setPosition(pos, QTextCursor::KeepAnchor);

    QTextCharFormat format;
    format.setFontWeight(weight);
    format.setForeground(color);

    cursor.mergeCharFormat(format);
    cursor.clearSelection();

    if (atEnd)
        scrollToEnd();

    if (_outputPane->isCollapsed() || _outputPane->activeBox() != OutputPane::Console)
        _outputPane->shine(OutputPane::Console);
}

void ConsoleBox::scrollToEnd()
{
    auto scrollBar = _textEdit->verticalScrollBar();
    scrollBar->setSliderPosition(scrollBar->maximum());
}

void ConsoleBox::fade()
{
    QTextDocument *document = _textEdit->document();
    QTextCursor cursor(document);

    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QTextCharFormat format;
    format.setForeground(QColor("#707477"));

    cursor.mergeCharFormat(format);
    cursor.clearSelection();
}

void ConsoleBox::reset()
{
    _textEdit->clear();
}

void ConsoleBox::onStandardError(const QString& output)
{
    printError(output);
}

void ConsoleBox::onStandardOutput(const QString& output)
{
   print(output);
}

