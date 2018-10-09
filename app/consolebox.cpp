#include <consolebox.h>
#include <runmanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <centralwidget.h>
#include <control.h>
#include <appfontsettings.h>
#include <qmlcodeeditor.h>
#include <qmlcodeeditorwidget.h>
#include <utilityfunctions.h>

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>

class PlainTextEdit : public QPlainTextEdit {
    friend class ConsoleBox;
};

ConsoleBox::ConsoleBox(QWidget* parent) : QWidget(parent)
{
    m_layout = new QVBoxLayout(this);
    m_textBrowser = new PlainTextEdit;

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_textBrowser);

    m_textBrowser->viewport()->setMouseTracking(true);
    m_textBrowser->viewport()->installEventFilter(this);
    m_textBrowser->setWordWrapMode(QTextOption::WordWrap);
    m_textBrowser->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    UtilityFunctions::adjustFontPixelSize(m_textBrowser, -1);

    connect(RunManager::instance(), &RunManager::standardError, this, &ConsoleBox::onStandardError);
    connect(RunManager::instance(), &RunManager::standardOutput, this, &ConsoleBox::onStandardOutput);
}

bool ConsoleBox::isClean() const
{
    return m_textBrowser->toPlainText().isEmpty();
}

void ConsoleBox::print(const QString& text)
{
    printFormatted(text, m_textBrowser->palette().text().color(), QFont::Normal);
}

void ConsoleBox::printError(const QString& text)
{
    printFormatted(text, "#B44B46", QFont::Normal);
}

void ConsoleBox::printFormatted(const QString& text, const QColor& color, QFont::Weight weight)
{
    const QScrollBar* bar = m_textBrowser->verticalScrollBar();
    const bool atEnd = bar->value() > bar->maximum() * 0.8;

    QTextCharFormat format;
    format.setFontWeight(weight);
    format.setForeground(color);

    QTextCursor cursor(m_textBrowser->textCursor());
    cursor.movePosition(QTextCursor::End);
    const int offset = cursor.position();
    cursor.insertText(text, format);

    QRegularExpression exp("[a-z_][a-zA-Z0-9_]+::[a-f0-9]+:.[\\w\\\\\\/\\.\\d]+:\\d+");
    QRegularExpressionMatchIterator i = exp.globalMatch(text);

    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        cursor.setPosition(offset + match.capturedStart());
        cursor.setPosition(offset + match.capturedEnd(), QTextCursor::KeepAnchor);

        QTextCharFormat format;
        format.setFontWeight(weight);
        format.setForeground(QColor("#025dbf"));
        format.setFontUnderline(true);
        cursor.mergeCharFormat(format);
    }

    if (atEnd)
        scrollToEnd();

    if (m_outputPane->isCollapsed() || m_outputPane->activeBox() != OutputPane::Console)
        m_outputPane->shine(OutputPane::Console);
}

void ConsoleBox::scrollToEnd()
{
    auto scrollBar = m_textBrowser->verticalScrollBar();
    scrollBar->setSliderPosition(scrollBar->maximum());
}

void ConsoleBox::onLinkClick(const QString& link)
{
    QRegularExpression exp("^[a-z_][a-zA-Z0-9_]+::([a-f0-9]+):.([\\w\\\\\\/\\.\\d]+):(\\d+)");
    const QString& uid = exp.match(link).captured(1);
    const QString& path = exp.match(link).captured(2);
    const int lineNumber = exp.match(link).captured(3).toInt();
    QmlCodeEditorWidget* editorWidget = WindowManager::mainWindow()->centralWidget()->qmlCodeEditorWidget();
    Control* control = nullptr;

    for (const auto c : Control::controls()) {
        if (c->uid() == uid)
            control = c;
    }

    if (!control)
        return;

    // FIXME: What if the file is a global resources file?
    editorWidget->openInternal(control, path);
    editorWidget->codeEditor()->gotoLine(lineNumber);
}

void ConsoleBox::fade()
{
    QTextCharFormat format;
    format.setForeground(QColor("#707477"));

    QTextCursor cursor(m_textBrowser->document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(format);
}

void ConsoleBox::sweep()
{
    m_textBrowser->clear();
}

void ConsoleBox::onStandardError(const QString& output)
{
    printError(output);
}

void ConsoleBox::onStandardOutput(const QString& output)
{
    print(output);
}

bool ConsoleBox::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_textBrowser->viewport()
            && (event->type() == QEvent::MouseMove
                || event->type() == QEvent::MouseButtonRelease)) {
        auto e = static_cast<QMouseEvent*>(event);
        auto ce = m_textBrowser;
        auto pos = ce->contentOffset() + e->pos();
        auto block = ce->firstVisibleBlock();
        auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
        auto bottom = top + ce->blockBoundingRect(block).height();

        while (block.isValid() && top <= rect().bottom()) {
            if (pos.y() >= top && pos.y() <= bottom) {
                QRegularExpression exp("[a-z_][a-zA-Z0-9_]+::[a-f0-9]+:.[\\w\\\\\\/\\.\\d]+:\\d+");
                if (exp.match(block.text()).hasMatch()
                        && pos.x() < m_textBrowser->fontMetrics().horizontalAdvance(exp.match(block.text()).captured())) {
                    if (event->type() == QEvent::MouseMove)
                        m_textBrowser->viewport()->setCursor(Qt::PointingHandCursor);
                    else
                        onLinkClick(exp.match(block.text()).captured());
                } else {
                    m_textBrowser->viewport()->setCursor(Qt::IBeamCursor);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + ce->document()->documentLayout()->blockBoundingRect(block).height();
        }
    }

    return false;
}

