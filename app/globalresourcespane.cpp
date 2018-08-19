#include <globalresourcespane.h>
#include <focuslesslineedit.h>

#include <QPainter>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QFileSystemModel>

namespace {
const int ROW_HEIGHT = 21;

void initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#a671bd");
    palette.setColor(QPalette::Dark, "#9968ad");
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, "#2d1f33");
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::WindowText, "#2d1f33");
    palette.setColor(QPalette::AlternateBase, "#f6f2f7");
    widget->setPalette(palette);
}
}

class GlobalListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit GlobalListDelegate(GlobalResourcesPane* parent) : QStyledItemDelegate(parent)
    {}

    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), ROW_HEIGHT);
    }
};

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : QTreeWidget(parent)
  , m_searchEdit(new FocuslessLineEdit(this))
  , m_fileSystemModel(new QFileSystemModel(this))
{
    initPalette(this);

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);
    header()->setSectionsMovable(false);

    headerItem()->setText(0, tr("Forms"));

    setColumnCount(1);
    setIndentation(0);
    setIconSize({15, 15});
    setAcceptDrops(true);
    setDragEnabled(false);
    setSortingEnabled(true);
    setRootIsDecorated(false);
    setItemsExpandable(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(new GlobalListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::SingleSelection);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(
                QString {
                    "QTreeView {"
                    "    border: 1px solid %1;"
                    "} QHeaderView::section {"
                    "    padding-left: 5px;"
                    "    color: %4;"
                    "    border: none;"
                    "    border-bottom: 1px solid %1;"
                    "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                    "                                stop:0 %2, stop:1 %3);"
                    "}"
                }
                .arg(palette().text().color().lighter(270).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name())
    );

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, &FocuslessLineEdit::textChanged, this, &GlobalResourcesPane::filterList);

    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
}

void GlobalResourcesPane::sweep()
{
    // TODO
    m_searchEdit->clear();
    clear();
}

void GlobalResourcesPane::filterList(const QString& filter)
{
    // TODO
}

void GlobalResourcesPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().text().color().lighter(270));
    lineColor.setAlpha(50);
    painter.setPen(lineColor);

    for (int i = 0; i < viewport()->height() / qreal(ROW_HEIGHT); ++i) {
        QRectF rect(0, i * ROW_HEIGHT, viewport()->width(), ROW_HEIGHT);
        QPainterPath path;
        path.addRect(rect);
        painter.setClipPath(path);

        // Fill background
        if (i % 2)
            painter.fillRect(rect, palette().alternateBase());

        // Draw top and bottom lines
        painter.drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
        painter.drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});
    }

    QTreeWidget::paintEvent(e);
}

void GlobalResourcesPane::updateGeometries()
{
    QTreeWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    QRect vg = viewport()->geometry();
    QRect geometryRect(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    setViewportMargins(vm);
    m_searchEdit->setGeometry(geometryRect);
}

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{310, 240};
}

#include "globalresourcespane.moc"
