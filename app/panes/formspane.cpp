#include <formspane.h>
#include <pushbutton.h>
#include <saveutils.h>
#include <projectmanager.h>
#include <designerscene.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <filesystemutils.h>

#include <QStandardPaths>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QDir>
#include <QTemporaryDir>

namespace {
bool isProjectStarted = false;
const int ROW_HEIGHT = 21;

void initPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Light, "#bf5861");
    palette.setColor(QPalette::Dark, "#b05159");
    palette.setColor(QPalette::AlternateBase, "#f7e6e8");
    widget->setPalette(palette);
}

void fillBackground(QPainter* painter, const QStyleOptionViewItem& option, int row)
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
    QColor lineColor(pal.dark().color());
    lineColor.setAlpha(50);
    painter->setPen(lineColor);
    painter->drawLine(rect.topLeft() + QPointF{0.5, 0.0}, rect.topRight() - QPointF{0.5, 0.0});
    painter->drawLine(rect.bottomLeft() + QPointF{0.5, 0.0}, rect.bottomRight() - QPointF{0.5, 0.0});

    painter->restore();
}
}

class FormsListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit FormsListDelegate(FormsPane* parent) : QStyledItemDelegate(parent)
      , m_formsPane(parent)
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

        fillBackground(painter, option, index.row());

        // Draw icon
        Q_ASSERT(UtilityFunctions::window(m_formsPane));
        const QPixmap& iconPixmap = icon.pixmap(UtilityFunctions::window(m_formsPane),
                                                option.decorationSize,
                                                isSelected ? QIcon::Selected : QIcon::Normal);
        painter->drawPixmap(iconRect, iconPixmap, iconPixmap.rect());

        if (isSelected)
            painter->setPen(option.palette.highlightedText().color());
        else
            painter->setPen(option.palette.text().color());

        // Draw text
        const QRectF& textRect = option.rect.adjusted(option.decorationSize.width() + 10, 0, 0, 0);
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
    FormsPane* m_formsPane;
};

FormsPane::FormsPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_addButton(new PushButton(this))
  , m_removeButton(new PushButton(this))
{
    initPalette(this);

    header()->setFixedHeight(23);
    header()->setDefaultSectionSize(1);
    header()->setMinimumSectionSize(1);

    headerItem()->setText(0, tr("Forms"));

    setColumnCount(1);
    setIndentation(0);
    setIconSize({15, 15});
    setDragEnabled(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(new FormsListDelegate(this));
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
                .arg(palette().dark().color().darker(140).name())
                .arg(palette().light().color().name())
                .arg(palette().dark().color().name())
                .arg(palette().brightText().color().name())
    );

    m_addButton->setCursor(Qt::PointingHandCursor);
    m_addButton->setToolTip(tr("Add new form to the project"));
    m_addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_addButton->setFixedSize(18, 18);
    m_addButton->setIconSize(QSize(12, 12));
    m_addButton->setIcon(QIcon(PaintUtils::renderOverlaidPixmap(":/images/plus.png", palette().text().color(), devicePixelRatioF())));
    connect(m_addButton, &PushButton::clicked, this, &FormsPane::onAddButtonClick);

    m_removeButton->setCursor(Qt::PointingHandCursor);
    m_removeButton->setToolTip(tr("Remove selected form from the project"));
    m_removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_removeButton->setFixedSize(18, 18);
    m_removeButton->setIconSize(QSize(12, 12));
    m_removeButton->setIcon(QIcon(PaintUtils::renderOverlaidPixmap(":/images/minus.png", palette().text().color(), devicePixelRatioF())));
    connect(m_removeButton, &PushButton::clicked, this, &FormsPane::onRemoveButtonClick);

    /*
        NOTE: No need to catch any signals about form creation or deletion from ControlRemovingManager
              or ControlCreatingManager since the member functions that are operating on forms are
              private members and only used by FormsPane.
    */

    connect(m_designerScene, &DesignerScene::currentFormChanged, this, &FormsPane::refresh); // FIXME: This function has severe performance issues.
    connect(this, &FormsPane::currentItemChanged, this, &FormsPane::onCurrentItemChange);
    connect(ProjectManager::instance(), &ProjectManager::started, this, [=] {
        Q_ASSERT(!isProjectStarted);
        isProjectStarted = true;
        refresh(); // FIXME: This function has severe performance issues.
    });
}

void FormsPane::discharge()
{
    isProjectStarted = false;
    blockSignals(true);
    clear();
    blockSignals(false);
}

void FormsPane::onAddButtonClick()
{
    QTemporaryDir temp;
    Q_ASSERT(temp.isValid());

    SaveUtils::initControlMeta(temp.path());

    const QString& thisDir = SaveUtils::toControlThisDir(temp.path());

    QDir(thisDir).mkpath(".");
    FileSystemUtils::copy(":/resources/qmls/form.qml", thisDir, true, true);
    QFile::rename(thisDir + "/form.qml", thisDir + '/' + SaveUtils::controlMainQmlFileName());
    ControlCreationManager::createForm(temp.path());

    refresh(); // FIXME: This function has severe performance issues.
}

void FormsPane::onRemoveButtonClick()
{
    if (topLevelItemCount() > 1) // FIXME
        ControlRemovingManager::removeControl(m_designerScene->currentForm(), true);
    // refresh(); // Not needed, m_designerScene already emits currentFormChanged signal
}

void FormsPane::onCurrentItemChange()
{
    Q_ASSERT(currentItem());

    const QString& id = currentItem()->text(0);
    for (Form* form : m_designerScene->forms()) {
        if (form->id() == id)
            m_designerScene->setCurrentForm(form);
    }
}

void FormsPane::refresh()
{
    if (!isProjectStarted)
        return;

    blockSignals(true);

    clear();

    QTreeWidgetItem* selectionItem = nullptr;
    // FIXME: Should we use scene->forms() instead? --but if you do make sure you order forms with their indexes--
    for (const QString& path : SaveUtils::formPaths(ProjectManager::dir())) {
        const QString& id = SaveUtils::controlId(path);
        Q_ASSERT(!id.isEmpty());

        auto item = new QTreeWidgetItem;
        item->setText(0, id);
        item->setIcon(0, PaintUtils::renderItemIcon(":/images/form.png", palette()));

        addTopLevelItem(item);

        if (m_designerScene->currentForm()->id() == id)
            selectionItem = item;
    }

    if (selectionItem)
        selectionItem->setSelected(true);

    blockSignals(false);
}

void FormsPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());
    painter.fillRect(rect(), palette().base());
    painter.setClipping(true);

    QColor lineColor(palette().dark().color());
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

void FormsPane::updateGeometries()
{
    QTreeWidget::updateGeometries();
    QRect vg = viewport()->geometry();
    m_addButton->move(vg.topRight() + QPoint(-38, -21));
    m_removeButton->move(vg.topRight() + QPoint(-19, -21));
}

QSize FormsPane::sizeHint() const
{
    return QSize{190, 190};
}

#include "formspane.moc"
