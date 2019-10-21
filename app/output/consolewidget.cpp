#include <consolewidget.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QScrollBar>
#include <QTextBlock>
#include <QLabel>
#include <QToolButton>
#include <QTimer>
#include <QLayout>
#include <QTextStream>
#include <QToolBar>

ConsoleWidget::ConsoleWidget(QWidget* parent) : QPlainTextEdit(parent)
  , m_toolBar(new QToolBar(this))
  , m_iconLabel(new QLabel(this))
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
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setObjectName("m_plainTextEdit");
    setStyleSheet("#m_plainTextEdit { border: 1px solid #c4c4c4;"
                  "border-top: none; border-bottom: none;}");
    setWordWrapMode(QTextOption::WordWrap);
    UtilityFunctions::adjustFontPixelSize(this, -1);

    m_toolBar->layout()->setSpacing(3);
    m_toolBar->layout()->setContentsMargins(1, 1, 1, 1);

    m_toolBar->setFixedHeight(20);
    m_toolBar->addWidget(UtilityFunctions::createSpacingWidget(QSize(1, 1)));
    m_toolBar->addWidget(m_iconLabel);
    m_toolBar->addWidget(m_titleLabel);
    m_toolBar->addSeparator();
    m_toolBar->addWidget(m_clearButton);
    m_toolBar->addWidget(m_fontSizeUpButton);
    m_toolBar->addWidget(m_fontSizeDownButton);
    m_toolBar->addWidget(UtilityFunctions::createSpacerWidget(Qt::Horizontal));
    m_toolBar->addWidget(m_minimizeButton);

    m_iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/designer/console.svg"), QSize(16, 16), this));
    m_iconLabel->setFixedHeight(16);

    m_titleLabel->setText(tr("Console"));
    m_titleLabel->setFixedSize(50, 18);

    m_clearButton->setFixedSize({18, 18});
    m_clearButton->setIcon(QIcon(":/images/designer/clear.svg"));
    m_clearButton->setToolTip(tr("Clean console output"));
    m_clearButton->setCursor(Qt::PointingHandCursor);

    m_fontSizeUpButton->setFixedSize({18, 18});
    m_fontSizeUpButton->setIcon(QIcon(":/images/designer/font-increase.svg"));
    m_fontSizeUpButton->setToolTip(tr("Increase font size"));
    m_fontSizeUpButton->setCursor(Qt::PointingHandCursor);

    m_fontSizeDownButton->setFixedSize({18, 18});
    m_fontSizeDownButton->setIcon(QIcon(":/images/designer/font-decrease.svg"));
    m_fontSizeDownButton->setToolTip(tr("Decrease font size"));
    m_fontSizeDownButton->setCursor(Qt::PointingHandCursor);

    m_minimizeButton->setFixedSize({18, 18});
    m_minimizeButton->setIcon(QIcon(":/images/designer/down.svg"));
    m_minimizeButton->setToolTip(tr("Minimize the pane"));
    m_minimizeButton->setCursor(Qt::PointingHandCursor);

    connect(m_fontSizeDownButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomOut
        UtilityFunctions::adjustFontPixelSize(this, -1);
        updateViewportMargins();
    });
    connect(m_minimizeButton, &QToolButton::clicked,
            this, &ConsoleWidget::minimized);
    connect(this, &ConsoleWidget::blockCountChanged,
            this, &ConsoleWidget::updateViewportMargins);
    connect(m_fontSizeUpButton, &QToolButton::clicked,
            this, [=] { // TODO: Change this with zoomIn
        UtilityFunctions::adjustFontPixelSize(this, 1);
        updateViewportMargins();
    });
    connect(m_clearButton, &QToolButton::clicked,
            this, &ConsoleWidget::clear);
    updateViewportMargins();
}

void ConsoleWidget::fade()
{
    QTextCharFormat faded;
    faded.setForeground(palette().brush(QPalette::Disabled, QPalette::Text));
    QTextCursor cursor(document());
    cursor.movePosition(QTextCursor::Start);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.mergeCharFormat(faded);
}

void ConsoleWidget::press(const QString& text, const QBrush& brush, QFont::Weight weight)
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
        const PathFinder::AssetsResult& assetsResult = PathFinder::findAssets(line);
        const PathFinder::DesignsResult& designsResult = PathFinder::findDesigns(line);

        if (cursor.position() != 0)
            cursor.insertBlock();

        if (assetsResult.isNull() && designsResult.isNull()) {
            cursor.insertText(line, format);
            continue;
        }

        Q_ASSERT(assetsResult.isNull() || designsResult.isNull());

        const PathFinder::Result& result = !assetsResult.isNull()
                ? static_cast<const PathFinder::Result&>(assetsResult)
                : static_cast<const PathFinder::Result&>(designsResult);
        cursor.insertText(line.mid(0, result.begin), format);
        cursor.insertText(line.mid(result.begin, result.length), linkFormat);
        cursor.insertText(line.mid(result.end), format);
    }

    if (scrollDown)
        verticalScrollBar()->setValue(verticalScrollBar()->maximum());

    if (!isVisible())
        emit flash();
}

void ConsoleWidget::onLinkClick(const PathFinder::Result& result)
{
    if (result.type == PathFinder::Result::Assets) {
        emit assetsFileOpened(result.relativePath, result.line, 0);
    } else {
        emit designsFileOpened(static_cast<const PathFinder::DesignsResult&>(result).control,
                                result.relativePath, result.line, 0);
    }
}

void ConsoleWidget::updateViewportMargins()
{
    QMargins vm = viewportMargins();
    vm.setTop(m_toolBar->height());
    setViewportMargins(vm);
}

bool ConsoleWidget::eventFilter(QObject* w, QEvent* e)
{
    if (w == viewport() && (e->type() == QEvent::MouseMove || e->type() == QEvent::MouseButtonRelease)) {
        QMouseEvent* event = static_cast<QMouseEvent*>(e);
        QTextBlock block = firstVisibleBlock();
        qreal top = blockBoundingGeometry(block).translated(contentOffset()).top();
        qreal bottom = top + blockBoundingRect(block).height();

        while (block.isValid() && top <= viewport()->rect().bottom()) {
            if (event->pos().y() >= top && event->pos().y() <= bottom) {
                const QString& line = block.text();
                const PathFinder::AssetsResult& assetsResult = PathFinder::findAssets(line);
                const PathFinder::DesignsResult& designsResult = PathFinder::findDesigns(line);

                if (assetsResult.isNull() && designsResult.isNull()) {
                    viewport()->setCursor(Qt::IBeamCursor);
                } else {
                    Q_ASSERT(assetsResult.isNull() || designsResult.isNull());

                    const PathFinder::Result& result = !assetsResult.isNull()
                            ? static_cast<const PathFinder::Result&>(assetsResult)
                            : static_cast<const PathFinder::Result&>(designsResult);
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

void ConsoleWidget::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);
    m_toolBar->setGeometry(0, 0, viewport()->width() + 2, m_toolBar->height());
}

QSize ConsoleWidget::minimumSizeHint() const
{
    return {0, 100};
}

QSize ConsoleWidget::sizeHint() const
{
    return {0, 100};
}
