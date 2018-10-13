#include <consolepane.h>
#include <runmanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <centralwidget.h>
#include <control.h>
#include <appfontsettings.h>
#include <qmlcodeeditor.h>
#include <qmlcodeeditorwidget.h>
#include <utilityfunctions.h>
#include <utilsicons.h>
#include <transparentstyle.h>

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QLabel>
#include <QToolButton>
#include <QTimer>

class PlainTextEdit : public QPlainTextEdit {
    explicit PlainTextEdit(QWidget* parent = nullptr) : QPlainTextEdit(parent) {}
    friend class ConsolePane;
};

ConsolePane::ConsolePane(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_plainTextEdit(new PlainTextEdit(this))
  , m_toolBar(new QToolBar(this))
  , m_titleLabel(new QLabel(this))
  , m_clearButton(new QToolButton(this))
  , m_fontSizeUpButton(new QToolButton(this))
  , m_fontSizeDownButton(new QToolButton(this))
  , m_minimizeButton(new QToolButton(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_layout->setSpacing(0);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->addWidget(m_toolBar);
    m_layout->addWidget(m_plainTextEdit);

    m_titleLabel->setText("   " + tr("Console Output") + "   ");
    m_titleLabel->setFixedHeight(20);

    TransparentStyle::attach(m_toolBar);
    m_toolBar->addWidget(m_titleLabel);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({4, 4}));
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(m_fontSizeUpButton);
    m_toolBar->addWidget(m_fontSizeDownButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_minimizeButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget({2, 2}));
    m_toolBar->setIconSize({14, 14});

    m_clearButton->setFixedSize({18, 18});
    m_clearButton->setIcon(Utils::Icons::CLEAN_TOOLBAR.icon());
    m_clearButton->setToolTip(tr("Clean console output"));
    m_clearButton->setCursor(Qt::PointingHandCursor);
    connect(m_clearButton, &QToolButton::clicked,
            m_plainTextEdit, &PlainTextEdit::clear);

    m_fontSizeUpButton->setFixedSize({18, 18});
    m_fontSizeUpButton->setIcon(Utils::Icons::PLUS_TOOLBAR.icon());
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(m_plainTextEdit, 1);
    });

    m_fontSizeDownButton->setFixedSize({18, 18});
    m_fontSizeDownButton->setIcon(Utils::Icons::MINUS.icon());
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(m_plainTextEdit, -1);
    });

    m_minimizeButton->setFixedSize({18, 18});
    m_minimizeButton->setIcon(Utils::Icons::CLOSE_SPLIT_BOTTOM.icon());
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &ConsolePane::minimized);

    m_plainTextEdit->setObjectName("m_plainTextEdit");
    m_plainTextEdit->setStyleSheet("#m_plainTextEdit { border: 1px solid #c4c4c4;"
                                   "border-top: none; border-bottom: none;}");
    m_plainTextEdit->viewport()->setMouseTracking(true);
    m_plainTextEdit->viewport()->installEventFilter(this);
    m_plainTextEdit->setWordWrapMode(QTextOption::WordWrap);
    m_plainTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    UtilityFunctions::adjustFontPixelSize(m_plainTextEdit, -1);

    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        m_toolBar->setContentsMargins(0, 0, 0, 0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        m_toolBar->layout()->setSpacing(0);
        m_toolBar->setFixedHeight(22);
    });

    connect(RunManager::instance(), &RunManager::standardError,
            this, &ConsolePane::onStandardError);
    connect(RunManager::instance(), &RunManager::standardOutput,
            this, &ConsolePane::onStandardOutput);
}

bool ConsolePane::isClean() const
{
    return m_plainTextEdit->toPlainText().isEmpty();
}

void ConsolePane::print(const QString& text)
{
    printFormatted(text, m_plainTextEdit->palette().text().color(), QFont::Normal);
}

void ConsolePane::printError(const QString& text)
{
    printFormatted(text, "#B44B46", QFont::Normal);
}

void ConsolePane::printFormatted(const QString& text, const QColor& color, QFont::Weight weight)
{
    const QScrollBar* bar = m_plainTextEdit->verticalScrollBar();
    const bool atEnd = bar->value() > bar->maximum() * 0.8;

    QTextCharFormat format;
    format.setFontWeight(weight);
    format.setForeground(color);

    QTextCursor cursor(m_plainTextEdit->textCursor());
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

     if (isHidden()) // FIXME: Add this to global settings
        emit flash();
}

void ConsolePane::scrollToEnd()
{
    auto scrollBar = m_plainTextEdit->verticalScrollBar();
    scrollBar->setSliderPosition(scrollBar->maximum());
}

void ConsolePane::onLinkClick(const QString& link)
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

void ConsolePane::fade()
{
    QTextCharFormat format;
    format.setForeground(QColor("#707477"));

    QTextCursor cursor(m_plainTextEdit->document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(format);
}

void ConsolePane::sweep()
{
    m_plainTextEdit->clear();
}

void ConsolePane::onStandardError(const QString& output)
{
    printError(output);
}

void ConsolePane::onStandardOutput(const QString& output)
{
    print(output);
}

bool ConsolePane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == m_plainTextEdit->viewport()
            && (event->type() == QEvent::MouseMove
                || event->type() == QEvent::MouseButtonRelease)) {
        auto e = static_cast<QMouseEvent*>(event);
        auto ce = m_plainTextEdit;
        auto pos = ce->contentOffset() + e->pos();
        auto block = ce->firstVisibleBlock();
        auto top = ce->blockBoundingGeometry(block).translated(ce->contentOffset()).top();
        auto bottom = top + ce->blockBoundingRect(block).height();

        while (block.isValid() && top <= rect().bottom()) {
            if (pos.y() >= top && pos.y() <= bottom) {
                QRegularExpression exp("[a-z_][a-zA-Z0-9_]+::[a-f0-9]+:.[\\w\\\\\\/\\.\\d]+:\\d+");
                if (exp.match(block.text()).hasMatch()
                        && pos.x() < m_plainTextEdit->fontMetrics().horizontalAdvance(exp.match(block.text()).captured())) {
                    if (event->type() == QEvent::MouseMove)
                        m_plainTextEdit->viewport()->setCursor(Qt::PointingHandCursor);
                    else
                        onLinkClick(exp.match(block.text()).captured());
                } else {
                    m_plainTextEdit->viewport()->setCursor(Qt::IBeamCursor);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + ce->document()->documentLayout()->blockBoundingRect(block).height();
        }
    }

    return false;
}

QSize ConsolePane::minimumSizeHint() const
{
    return {0, 100};
}

QSize ConsolePane::sizeHint() const
{
    return {0, 100};
}
