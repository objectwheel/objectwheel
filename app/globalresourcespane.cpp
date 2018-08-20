#include <globalresourcespane.h>
#include <focuslesslineedit.h>
#include <projectmanager.h>
#include <saveutils.h>
#include <filemanager.h>
#include <wfw.h>

#include <QPainter>
#include <QHeaderView>
#include <QStyledItemDelegate>
#include <QFileSystemModel>

namespace {
const int ROW_HEIGHT = 21;

void initPalette(QWidget* widget)
{    
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#bf5861");
    palette.setColor(QPalette::Dark, "#b05159");
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, "#331719");
    palette.setColor(QPalette::BrightText, Qt::white);
    palette.setColor(QPalette::WindowText, "#331719");
    palette.setColor(QPalette::AlternateBase, "#f7e6e8");
    widget->setPalette(palette);
}

void fillBackground(QPainter* painter, const QStyleOptionViewItem& option, int row, bool verticalLine)
{
    painter->save();

    bool isSelected = option.state & QStyle::State_Selected;
    const QPalette& pal = option.palette;
    const QRectF& rect = option.rect;

    QPainterPath path;
    path.addRect(rect);
    painter->setClipPath(path);
    painter->setClipping(true);

    // Fill background
    if (isSelected) {
        painter->fillRect(rect, pal.highlight());
    } else {
        if (row % 2)
            painter->fillRect(rect, pal.alternateBase());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    QColor lineColor(isSelected ? pal.highlightedText().color() : pal.text().color().lighter(210));
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    // Draw vertical line
    if (verticalLine) {
        painter->drawLine(rect.topRight() + QPointF(-0.5, 0.5),
                          rect.bottomRight() + QPointF(-0.5, -0.5));
    }

    painter->restore();
}
}

class GlobalListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit GlobalListDelegate(GlobalResourcesPane* parent) : QStyledItemDelegate(parent)
      , m_globalPane(parent)
    {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override
    {
        painter->save();
        painter->setRenderHint(QPainter::Antialiasing);

        const bool isSelected = option.state & QStyle::State_Selected;
        const QAbstractItemModel* model = index.model();
        const QIcon& icon = model->data(index, Qt::DecorationRole).value<QIcon>();

        QRectF iconRect({}, QSizeF{option.decorationSize});
        iconRect.moveCenter(option.rect.center());
        iconRect.moveLeft(option.rect.left() + 5);

        fillBackground(painter, option,
                       /*calculateVisibleRow(m_globalPane->itemFromIndex(index), m_globalPane)*/ m_globalPane->d_func()->viewIndex(index),
                       index.column() == 0);

        // Draw icon
        const QPixmap& iconPixmap = icon.pixmap(wfw(m_globalPane), option.decorationSize,
                                                isSelected ? QIcon::Selected : QIcon::Normal);
        painter->drawPixmap(iconRect, iconPixmap, iconPixmap.rect());

        // Draw text
        if (model->data(index, Qt::UserRole).toBool())
            painter->setPen(option.palette.link().color());
        else if (isSelected)
            painter->setPen(option.palette.highlightedText().color());
        else
            painter->setPen(option.palette.text().color());

        QRectF textRect;
        if (index.column() == 0)
            textRect = option.rect.adjusted(option.decorationSize.width() + 10, 0, 0, 0);
        else
            textRect = option.rect.adjusted(5, 0, 0, 0);

        const QString& text = index.data(Qt::DisplayRole).toString();
        painter->drawText(textRect,
                          option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width()),
                          QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }


    QSize sizeHint(const QStyleOptionViewItem& opt, const QModelIndex& index) const override
    {
        const QSize& size = QStyledItemDelegate::sizeHint(opt, index);
        return QSize(size.width(), ROW_HEIGHT);
    }

private:
    GlobalResourcesPane* m_globalPane;
};

GlobalResourcesPane::GlobalResourcesPane(QWidget* parent) : QTreeView(parent)
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

    setIndentation(16);
    setIconSize({15, 15});
    setAcceptDrops(true);
    setDragEnabled(false);
    setSortingEnabled(false);
    setRootIsDecorated(true);
    setItemsExpandable(true);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
    setItemDelegate(new GlobalListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeView::SelectRows);
    setSelectionMode(QTreeView::SingleSelection);
    setDragDropMode(QAbstractItemView::NoDragDrop);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setVerticalScrollMode(QTreeView::ScrollPerPixel);
    setHorizontalScrollMode(QTreeView::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setStyleSheet(
                QString {
                    "QTreeView {"
                    "    border: 1px solid %1;"
                    "} QHeaderView::section {"
                    "    padding-left: 0px;"
                    "    padding-top: 5px;"
                    "    padding-bottom: 5px;"
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


    m_fileSystemModel->setFilter(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, &FocuslessLineEdit::textChanged, this, &GlobalResourcesPane::filterList);

    connect(ProjectManager::instance(), &ProjectManager::started,
            this, &GlobalResourcesPane::onProjectStart);
}

void GlobalResourcesPane::sweep()
{
    // TODO
    m_searchEdit->clear();
    setModel(nullptr);
}

void GlobalResourcesPane::onProjectStart()
{
    m_fileSystemModel->setRootPath(SaveUtils::toGlobalDir(ProjectManager::dir()));
    setModel(m_fileSystemModel);
    setRootIndex(m_fileSystemModel->index(m_fileSystemModel->rootPath()));
    hideColumn(1);
    hideColumn(2);
    hideColumn(3);
}

void GlobalResourcesPane::filterList(const QString& /*filter*/)
{
    // TODO
}

void GlobalResourcesPane::drawBranches(QPainter* painter, const QRect& rect,
                                       const QModelIndex& index) const
{
    Q_D(const GlobalResourcesPane);
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);

    const qreal width = 10;
    const QTreeViewItem &viewItem = d->viewItems.at(d->current);
    const bool hasChild = viewItem.hasChildren;
    const bool isSelected = d->selectionModel->isSelected(index);

    QRectF handleRect(0, 0, width, width);
    handleRect.moveCenter(rect.center());
    handleRect.moveRight(rect.right() - 0.5);

    QStyleOptionViewItem option;
    option.initFrom(this);
    option.rect = rect;
    if (isSelected)
        option.state |= QStyle::State_Selected;
    else if (option.state & QStyle::State_Selected)
        option.state ^= QStyle::State_Selected;

    fillBackground(painter, option, d->viewIndex(index), false);

    // Draw handle
    if (hasChild) {
        QPen pen;
        pen.setWidthF(1.2);
        pen.setColor(isSelected ? palette().highlightedText().color() : palette().text().color());
        painter->setPen(pen);
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(handleRect, 0, 0);

        painter->drawLine(QPointF(handleRect.left() + 2.5, handleRect.center().y()),
                          QPointF(handleRect.right() - 2.5, handleRect.center().y()));

        if (!isExpanded(index)) {
            painter->drawLine(QPointF(handleRect.center().x(), handleRect.top() + 2.5),
                              QPointF(handleRect.center().x(), handleRect.bottom() - 2.5));
        }
    }

    painter->restore();
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

    QTreeView::paintEvent(e);
}

void GlobalResourcesPane::updateGeometries()
{
    QTreeView::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    QRect vg = viewport()->geometry();
    QRect geometryRect(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    setViewportMargins(vm);
    m_searchEdit->setGeometry(geometryRect);
}

QSize GlobalResourcesPane::sizeHint() const
{
    return QSize{210, 340};
}

#include "globalresourcespane.moc"
