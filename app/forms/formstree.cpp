#include <formstree.h>
#include <formsdelegate.h>

#include <QHeaderView>
#include <QPainter>
#include <QApplication>

FormsTree::FormsTree(QWidget* parent) : QTreeWidget(parent)
  , m_delegate(new FormsDelegate(this))
{
    header()->setFixedHeight(20);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);

    headerItem()->setText(0, tr("Forms"));

    setColumnCount(1);
    setIndentation(0);
    setDragEnabled(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(m_delegate);
    setFocusPolicy(Qt::NoFocus);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::SingleSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto updatePalette = [=] {
        QPalette p(palette());
        p.setColor(QPalette::Light, "#bf5861");
        p.setColor(QPalette::Dark, "#b05159");
        p.setColor(QPalette::AlternateBase, "#f7e6e8");
        setPalette(p);
        setStyleSheet(QString { R"qss(
                        QTreeView {
                            border: 1px solid %1;
                        } QHeaderView::section {
                            padding-left: 5px;
                            color: %4;
                            border: none;
                            border-bottom: 1px solid %1;
                            background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,
                                                        stop:0 %2, stop:1 %3);
                        }
                    )qss"}
                    .arg(palette().dark().color().darker(140).name())
                    .arg(palette().light().color().name())
                    .arg(palette().dark().color().name())
                    .arg(palette().brightText().color().name())
        );
    };
    connect(qApp, &QApplication::paletteChanged, this, updatePalette);
    updatePalette();
}

FormsDelegate* FormsTree::delegate() const
{
    return m_delegate;
}

void FormsTree::paintEvent(QPaintEvent* e)
{
//    Q_D(const QTreeWidget);

//    QPainter painter(viewport());
//    painter.fillRect(rect(), palette().base());

//    QColor lineColor(palette().dark().color());
//    lineColor.setAlpha(50);
//    painter.setPen(lineColor);

//    qreal height = d->defaultItemHeight;
//    qreal rowCount = viewport()->height() / height;
//    for (int i = 0; i < rowCount; ++i) {
//        QRectF rect(0, i * height, viewport()->width(), height);
//        painter.setClipRect(rect);

//        // Fill background
//        if (i % 2)
//            painter.fillRect(rect, palette().alternateBase());

//        // Draw vertical line
//        QRectF cell(rect);
//        cell.setSize(QSizeF(header()->sectionSize(0), rect.height()));
//        painter.drawLine(cell.topRight() + QPointF(-0.5, 0.5),
//                         cell.bottomRight() + QPointF(-0.5, -0.5));

//        // Draw top and bottom lines
//        painter.drawLine(rect.topLeft() + QPointF(0.5, 0.0),
//                         rect.topRight() - QPointF(0.5, 0.0));
//        painter.drawLine(rect.bottomLeft() + QPointF(0.5, 0.0),
//                         rect.bottomRight() - QPointF(0.5, 0.0));
//    }

//    QTreeWidget::paintEvent(e);





//    QPainter painter(viewport());
//    painter.fillRect(rect(), palette().base());
//    painter.setClipping(true);

//    QColor lineColor(palette().dark().color());
//    lineColor.setAlpha(50);
//    painter.setPen(lineColor);

//    for (int i = 0; i < viewport()->height() / qreal(ROW_HEIGHT); ++i) {
//        QRectF rect(0, i * ROW_HEIGHT, viewport()->width(), ROW_HEIGHT);
//        QPainterPath path;
//        path.addRect(rect);
//        painter.setClipPath(path);

//        // Fill background
//        if (i % 2)
//            painter.fillRect(rect, palette().alternateBase());

//        // Draw top and bottom lines
//        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
//        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
//    }

    QTreeWidget::paintEvent(e);
}
