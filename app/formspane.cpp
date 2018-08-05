#include <formspane.h>
#include <flatbutton.h>
#include <toolboxtree.h>
#include <saveutils.h>
#include <parserutils.h>
#include <projectmanager.h>
#include <designerscene.h>
#include <filemanager.h>
#include <delayer.h>
#include <controlcreationmanager.h>
#include <controlremovingmanager.h>
#include <wfw.h>
#include <paintutils.h>

#include <QLabel>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPalette>
#include <QListWidget>
#include <QScrollBar>
#include <QPainter>
#include <QStyledItemDelegate>
#include <QHeaderView>
#include <QBitmap>

namespace {

void setPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Base, Qt::white);
    palette.setColor(QPalette::Text, "#401d20");
    palette.setColor(QPalette::Window, "#fcebed");
    palette.setColor(QPalette::WindowText, "#401d20");
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
            painter->fillRect(rect, pal.window());
        else
            painter->fillRect(rect, pal.base());
    }

    // Draw top and bottom lines
    painter->setPen("#30a14a51");
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
        const QPixmap& iconPixmap = icon.pixmap(wfw(m_formsPane), option.decorationSize,
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
        return QSize(size.width(), 21); // Fixed height for rows, 21
    }

private:
    FormsPane* m_formsPane;
};

FormsPane::FormsPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_addButton(new FlatButton(this))
  , m_removeButton(new FlatButton(this))
{
    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    ::setPalette(this);
    header()->setFont(fontMedium);
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
    setStyleSheet("QTreeView {"
                  "    border: 1px solid #a14a51;"
                  "    outline: 0;"
                  "} QHeaderView::section {"
                  "    padding-left: 5px;"
                  "    color: white;"
                  "    border: none;"
                  "    border-bottom: 1px solid #a14a51;"
                  "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                                stop:0 #bf5861, stop:1 #b3525a);"
                  "}");

    m_addButton->settings().topColor = "#62A558";
    m_addButton->settings().bottomColor = "#599750";
    m_addButton->settings().borderRadius = 10;
    m_addButton->settings().textColor = Qt::white;
    m_addButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_addButton->setFixedSize(20, 20);
    m_addButton->setIconSize(QSize(12, 12));
    m_addButton->setIcon(QIcon(":/images/plus.png"));
    connect(m_addButton, &FlatButton::clicked, this, &FormsPane::onAddButtonClick);

    m_removeButton->settings().topColor = "#C2504B";
    m_removeButton->settings().bottomColor = "#B34B46";
    m_removeButton->settings().borderRadius = 12;
    m_removeButton->settings().textColor = Qt::white;
    m_removeButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_removeButton->setFixedSize(20, 20);
    m_removeButton->setIconSize(QSize(12, 12));
    m_removeButton->setIcon(QIcon(":/images/minus.png"));
    connect(m_removeButton, &FlatButton::clicked, this, &FormsPane::onRemoveButtonClick);


    connect(ProjectManager::instance(), SIGNAL(started()), SLOT(onDatabaseChange()));
    //    FIXME: connect(SaveManager::instance(), SIGNAL(databaseChanged()), SLOT(onDatabaseChange()));
    connect(m_designerScene, SIGNAL(currentFormChanged(Form*)), SLOT(onDatabaseChange()));
    connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), SLOT(onCurrentFormChange()));
}

void FormsPane::paintEvent(QPaintEvent* e)
{
    QPainter painter(viewport());

    /* Fill background */
    const qreal bandHeight = topLevelItemCount() ? rowHeight(indexFromItem(topLevelItem(0))) : 21;
    const qreal bandCount = viewport()->height() / bandHeight;

    painter.fillRect(rect(), palette().base());

    for (int i = 0; i < bandCount; ++i) {
        if (i % 2)
            painter.fillRect(0, i * bandHeight, viewport()->width(), bandHeight, palette().window());
    }

    QTreeWidget::paintEvent(e);
}

void FormsPane::onRemoveButtonClick()
{
    ControlRemovingManager::removeForm(m_designerScene->currentForm());
}

void FormsPane::onAddButtonClick()
{
    auto tempPath = QStandardPaths::standardLocations(QStandardPaths::TempLocation)[0];
    tempPath = tempPath + separator() + "Objectwheel";

    rm(tempPath);

    if (!mkdir(tempPath) || !cp(":/resources/qmls/form", tempPath, true, true))
        return;

    ControlCreationManager::createForm(tempPath);

    rm(tempPath);
}

void FormsPane::onDatabaseChange()
{
    QIcon formIcon, mFormIcon;
    mFormIcon.addPixmap(PaintUtils::maskedPixmap(":/images/mform.png",
                                                 palette().text().color(),
                                                 this), QIcon::Normal);
    mFormIcon.addPixmap(PaintUtils::maskedPixmap(":/images/mform.png",
                                                 palette().highlightedText().color(),
                                                 this), QIcon::Selected);
    formIcon.addPixmap(PaintUtils::maskedPixmap(":/images/form.png",
                                                palette().text().color(),
                                                this), QIcon::Normal);
    formIcon.addPixmap(PaintUtils::maskedPixmap(":/images/form.png",
                                                palette().highlightedText().color(),
                                                this), QIcon::Selected);

    int row = -1;
    QString id;
    if (currentItem())
        id = currentItem()->text(0);

    clear();

    for (auto path : SaveUtils::formPaths(ProjectManager::dir())) {
        auto _id = ParserUtils::id(SaveUtils::toUrl(path));
        if (id == _id)
            row = topLevelItemCount();

        auto item = new QTreeWidgetItem;
        item->setText(0, _id);
        if (SaveUtils::isMain(path))
            item->setIcon(0, mFormIcon);
        else
            item->setIcon(0, formIcon);
        addTopLevelItem(item);
    }
    // FIXME   setCurrentRow(row);
}

void FormsPane::onCurrentFormChange()
{
    if (!currentItem())
        return;

    auto id = currentItem()->text(0);
    for (auto form : m_designerScene->forms()) {
        if (form->id() == id)
            m_designerScene->setCurrentForm(form);
    }
}

void FormsPane::updateGeometries()
{
    QTreeWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_addButton->height());
    QRect vg = viewport()->geometry();
    QRect geometryRect(vg.left(), vg.bottom(), vg.width(), m_addButton->height());
    setViewportMargins(vm);
    m_addButton->move(geometryRect.topLeft());
    m_removeButton->move(geometryRect.topRight() - QPoint(m_removeButton->width(), 0));
}

void FormsPane::sweep()
{
    clear();
}

QSize FormsPane::sizeHint() const
{
    return QSize{190, 160};
}

#include "formspane.moc"
