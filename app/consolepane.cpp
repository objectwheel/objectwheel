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
#include <pathfinder.h>

#include <QScrollBar>
#include <QRegularExpression>
#include <QMouseEvent>
#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QLabel>
#include <QToolButton>
#include <QTimer>
#include <QLayout>
#include <QTextStream>

ConsolePane::ConsolePane(QWidget* parent) : QPlainTextEdit(parent)
  , m_toolBar(new QToolBar(this))
  , m_titleLabel(new QLabel(this))
  , m_clearButton(new QToolButton(this))
  , m_fontSizeUpButton(new QToolButton(this))
  , m_fontSizeDownButton(new QToolButton(this))
  , m_minimizeButton(new QToolButton(this))
{
    viewport()->setMouseTracking(true);
    viewport()->installEventFilter(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setWordWrapMode(QTextOption::WordWrap);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setObjectName("m_plainTextEdit");
    setStyleSheet("#m_plainTextEdit { border: 1px solid #c4c4c4;"
                  "border-top: none; border-bottom: none;}");
    UtilityFunctions::adjustFontPixelSize(this, -1);

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
            this, &ConsolePane::clear);

    m_fontSizeUpButton->setFixedSize({18, 18});
    m_fontSizeUpButton->setIcon(Utils::Icons::PLUS_TOOLBAR.icon());
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(this, 1);
        updateViewportMargins();
    });

    m_fontSizeDownButton->setFixedSize({18, 18});
    m_fontSizeDownButton->setIcon(Utils::Icons::MINUS.icon());
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);
    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(this, -1);
        updateViewportMargins();
    });

    m_minimizeButton->setFixedSize({18, 18});
    m_minimizeButton->setIcon(Utils::Icons::CLOSE_SPLIT_BOTTOM.icon());
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &ConsolePane::minimized);
    connect(RunManager::instance(), &RunManager::standardErrorOutput,
             this, [=] (const QString& output) { press(output, palette().linkVisited()); });
    connect(RunManager::instance(), &RunManager::standardOutput,
            this, [=] (const QString& output) { press(output); });
    connect(this, &ConsolePane::blockCountChanged,
            this, &ConsolePane::updateViewportMargins);

    QTimer::singleShot(200, [=] { // Workaround for QToolBarLayout's obsolote serMargin function usage
        m_toolBar->setContentsMargins(0, 0, 0, 0);
        m_toolBar->layout()->setContentsMargins(0, 0, 0, 0); // They must be all same
        m_toolBar->layout()->setSpacing(0);
        m_toolBar->setFixedHeight(22);
        updateViewportMargins();
    });
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
    QTextCharFormat faded;
    faded.setForeground(palette().brush(QPalette::Disabled, QPalette::Text));
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(faded);
}

void ConsolePane::sweep()
{
    clear();
}

void ConsolePane::press(const QString& text, const QBrush& brush, QFont::Weight weight)
{
    bool scrollDown = verticalScrollBar()->value() == verticalScrollBar()->maximum();

    QTextCharFormat format;
    format.setFontWeight(weight);
    format.setForeground(brush);

    QTextCharFormat linkFormat;
    linkFormat.setFontWeight(weight);
    linkFormat.setForeground(palette().link());
    linkFormat.setFontUnderline(true);

    QTextCursor cursor(textCursor());
    cursor.movePosition(QTextCursor::End);

    QString line;
    QTextStream stream(text.toUtf8());

    while (stream.readLineInto(&line)) {
        line = PathFinder::cleansed(line, true);
        PathFinder::GlobalResult globalResult = PathFinder::findGlobal(line);
        PathFinder::InternalResult internalResult = PathFinder::findInternal(line);

        if (cursor.position() != 0)
            cursor.insertBlock();

        if (globalResult.isNull() && internalResult.isNull()) {
            cursor.insertText(line, format);
            continue;
        }

        Q_ASSERT(globalResult.isNull() || internalResult.isNull());

        if (!globalResult.isNull()) {
            cursor.insertText(line.mid(0, globalResult.begin), format);
            cursor.insertText(line.mid(globalResult.begin, globalResult.length), linkFormat);
            cursor.insertText(line.mid(globalResult.end), format);
        }

        if (!internalResult.isNull()) {
            cursor.insertText(line.mid(0, internalResult.begin), format);
            cursor.insertText(line.mid(internalResult.begin, internalResult.length), linkFormat);
            cursor.insertText(line.mid(internalResult.end), format);
        }
    }

    if (scrollDown)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

     if (isHidden())
        emit flash();
}

void ConsolePane::updateViewportMargins()
{
    QMargins vm = viewportMargins();
    vm.setTop(m_toolBar->height());
    setViewportMargins(vm);
}

bool ConsolePane::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == viewport()
            && (event->type() == QEvent::MouseMove
                || event->type() == QEvent::MouseButtonRelease)) {
        auto e = static_cast<QMouseEvent*>(event);
        auto pos = contentOffset() + e->pos();
        auto block = firstVisibleBlock();
        auto top = blockBoundingGeometry(block).translated(contentOffset()).top();
        auto bottom = top + blockBoundingRect(block).height();

        while (block.isValid() && top <= rect().bottom()) {
            if (pos.y() >= top && pos.y() <= bottom) {
                QRegularExpression exp("[a-z_][a-zA-Z0-9_]+::[a-f0-9]+:.[\\w\\\\\\/\\.\\d]+:\\d+");
                if (exp.match(block.text()).hasMatch()
                        && pos.x() < fontMetrics().horizontalAdvance(exp.match(block.text()).captured())) {
                    if (event->type() == QEvent::MouseMove)
                        viewport()->setCursor(Qt::PointingHandCursor);
                    else
                        onLinkClick(exp.match(block.text()).captured());
                } else {
                    viewport()->setCursor(Qt::IBeamCursor);
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + document()->documentLayout()->blockBoundingRect(block).height();
        }
    }

    return false;
}

void ConsolePane::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);
    m_toolBar->setGeometry(0, 0, viewport()->width() + 2, m_toolBar->height());
}

QSize ConsolePane::minimumSizeHint() const
{
    return {0, 100};
}

QSize ConsolePane::sizeHint() const
{
    return {0, 100};
}
