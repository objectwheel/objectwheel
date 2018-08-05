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

namespace {

void setPalette(QWidget* widget)
{
    QPalette palette(widget->palette());
    palette.setColor(QPalette::Text, "#401d20");
    palette.setColor(QPalette::WindowText, "#401d20");
    widget->setPalette(palette);
}
}

class FormListDelegate: public QStyledItemDelegate
{
    Q_OBJECT

public:
    FormListDelegate(QWidget* parent);
    void paint(QPainter* painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

FormListDelegate::FormListDelegate(QWidget* parent)
    : QStyledItemDelegate(parent)
{
}

void FormListDelegate::paint(QPainter* painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    const QAbstractItemModel* model = index.model();
    Q_ASSERT(model);
    painter->setRenderHint(QPainter::Antialiasing);

    const QPen oldPen = painter->pen();
    painter->setPen("#10000000");
    painter->drawLine(QPointF(0.5, option.rect.bottom() + 0.5),
                      QPointF(option.rect.right() + 0.5, option.rect.bottom() + 0.5));
    painter->setPen(oldPen);

    QStyledItemDelegate::paint(painter, option, index);
}

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
    setIconSize(QSize(14, 14));
    setDragEnabled(false);
    setRootIsDecorated(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(false);
    setItemDelegate(new FormListDelegate(this));
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
    if (topLevelItemCount() > 0) {
        const auto tli = topLevelItem(0);
        const auto& tlir = visualItemRect(tli);
        const qreal ic = (
                    viewport()->height() +
                    qAbs(tlir.y())
                    ) / (qreal) tlir.height();

        for (int i = 0; i < ic; i++) {
            if (i % 2 == 0) {
                painter.fillRect(
                            0,
                            tlir.y() + i * tlir.height(),
                            viewport()->width(),
                            tlir.height(),
                            QColor("#fae8ea")
                            );
            }
        }
    } else {
        const qreal hg = 20.0;
        const qreal ic = viewport()->height() / hg;

        for (int i = 0; i < ic; i++) {
            if (i % 2 == 0) {
                painter.fillRect(
                            0, i * hg,
                            viewport()->width(),
                            hg, QColor("#fae8ea")
                            );
            } else if (i == int(ic / 2.0) || i == int(ic / 2.0) + 1) {
                painter.setPen(QColor("#a5aab0"));
                painter.drawText(0, i * hg, viewport()->width(),
                                 hg, Qt::AlignCenter, "No forms to show");
            }
        }
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
            item->setIcon(0, QIcon(":/images/mform.png"));
        else
            item->setIcon(0, QIcon(":/images/form.png"));
        addTopLevelItem(item);
    }
// FIXME   setCurrentRow(row);
}

void FormsPane::onCurrentFormChange()
{
    if (!currentItem())
        return;

    auto id = currentItem()->text(0);
    for (auto form : m_designerScene->forms())
        if (form->id() == id)
            m_designerScene->setCurrentForm(form);
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
