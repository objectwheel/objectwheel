#include <consolepane.h>
#include <utilityfunctions.h>
#include <utilsicons.h>
#include <transparentstyle.h>

#include <QScrollBar>
#include <QTextBlock>
#include <QLabel>
#include <QToolButton>
#include <QTimer>
#include <QLayout>
#include <QTextStream>
#include <QToolBar>

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

void ConsolePane::fade()
{
    QTextCharFormat faded;
    faded.setForeground(palette().brush(QPalette::Disabled, QPalette::Text));
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(faded);
}

void ConsolePane::discharge()
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
        const PathFinder::GlobalResult& globalResult = PathFinder::findGlobal(line);
        const PathFinder::InternalResult& internalResult = PathFinder::findInternal(line);

        if (cursor.position() != 0)
            cursor.insertBlock();

        if (globalResult.isNull() && internalResult.isNull()) {
            cursor.insertText(line, format);
            continue;
        }

        Q_ASSERT(globalResult.isNull() || internalResult.isNull());

        const PathFinder::Result& result = !globalResult.isNull()
                ? static_cast<const PathFinder::Result&>(globalResult)
                : static_cast<const PathFinder::Result&>(internalResult);
        cursor.insertText(line.mid(0, result.begin), format);
        cursor.insertText(line.mid(result.begin, result.length), linkFormat);
        cursor.insertText(line.mid(result.end), format);
    }

    if (scrollDown)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

    if (isHidden())
        emit flash();
}

void ConsolePane::onLinkClick(const PathFinder::Result& result)
{
    if (result.type == PathFinder::Result::Global) {
        emit globalFileOpened(result.relativePath, result.line, 0);
    } else {
        emit internalFileOpened(static_cast<const PathFinder::InternalResult&>(result).control,
                                result.relativePath, result.line, 0);
    }
}

void ConsolePane::updateViewportMargins()
{
    QMargins vm = viewportMargins();
    vm.setTop(m_toolBar->height());
    setViewportMargins(vm);
}

bool ConsolePane::eventFilter(QObject* w, QEvent* e)
{
    if (w == viewport() && (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonRelease)) {
        QMouseEvent* event = static_cast<QMouseEvent*>(e);
        QTextBlock block = firstVisibleBlock();
        qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
        qreal bottom = top + blockBoundingRect(block).height();

        while (block.isValid() && top <= viewport()->rect().bottom()) {
            if (event->pos().y() >= top && event->pos().y() <= bottom) {
                const QString& line = block.text();
                const PathFinder::GlobalResult& globalResult = PathFinder::findGlobal(line);
                const PathFinder::InternalResult& internalResult = PathFinder::findInternal(line);

                if (globalResult.isNull() && internalResult.isNull()) {
                    viewport()->setCursor(Qt::IBeamCursor);
                } else {
                    Q_ASSERT(globalResult.isNull() || internalResult.isNull());

                    const PathFinder::Result& result = !globalResult.isNull()
                            ? static_cast<const PathFinder::Result&>(globalResult)
                            : static_cast<const PathFinder::Result&>(internalResult);
                    int begin = fontMetrics().horizontalAdvance(line.mid(0, result.begin));
                    int end = fontMetrics().horizontalAdvance(line.mid(0, result.end));

                    if (event->pos().x() >= begin && event->pos().x() <= end) {
                        if (event->type() == QEvent::MouseMove)
                            viewport()->setCursor(Qt::PointingHandCursor);
                        else
                            onLinkClick(result);
                    } else {
                        viewport()->setCursor(Qt::IBeamCursor);
                    }
                }
            }

            block = block.next();
            top = bottom;
            bottom = top + blockBoundingRect(block).height();
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
