#include <propertiespane.h>
#include <lineedit.h>
#include <saveutils.h>
#include <controlpropertymanager.h>
#include <designerscene.h>
#include <parserutils.h>
#include <transparentstyle.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <form.h>
#include <propertiestree.h>

#include <QStyledItemDelegate>
#include <QPainter>
#include <QHeaderView>
#include <QScrollBar>
#include <QCheckBox>
#include <QToolButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QFontDatabase>
#include <QMetaEnum>
#include <QComboBox>
#include <QHBoxLayout>
#include <QApplication>

PropertiesPane::PropertiesPane(QWidget* parent) : QWidget(parent)
  , m_propertiesTree(new PropertiesTree(this))
  , m_searchEdit(new LineEdit(this))
  , m_typeItem(new QTreeWidgetItem)
  , m_uidItem(new QTreeWidgetItem)
  , m_idItem(new QTreeWidgetItem)
  , m_indexItem(new QTreeWidgetItem)
  , m_idEdit(new QLineEdit(this))
  , m_indexEdit(new QSpinBox(this))
{
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setPlaceholderText(tr("Search"));
    m_searchEdit->addAction(PaintUtils::renderOverlaidPixmap(":/images/search.svg", "#595959",
                                                             m_searchEdit->devicePixelRatioF()),
                            QLineEdit::LeadingPosition);

    m_idEdit->setValidator(new QRegExpValidator(QRegExp("([a-z_][a-zA-Z0-9_]+)?"), m_idEdit));
    m_idEdit->setStyleSheet("QLineEdit { border: none; background: transparent; }");
    m_idEdit->setAttribute(Qt::WA_MacShowFocusRect, false);
    m_idEdit->setFocusPolicy(Qt::StrongFocus);
    m_idEdit->setSizePolicy(QSizePolicy::Ignored, m_idEdit->sizePolicy().verticalPolicy());
    m_idEdit->setMinimumWidth(1);

    TransparentStyle::attach(m_indexEdit);
    m_indexEdit->setCursor(Qt::PointingHandCursor);
    m_indexEdit->setFocusPolicy(Qt::StrongFocus);
    m_indexEdit->setSizePolicy(QSizePolicy::Ignored, m_indexEdit->sizePolicy().verticalPolicy());
    m_indexEdit->setMinimumWidth(1);
    m_indexEdit->setMaximum(std::numeric_limits<int>::max());
    m_indexEdit->setMinimum(std::numeric_limits<int>::lowest());
    UtilityFunctions::disableWheelEvent(m_indexEdit);

    m_typeItem->setText(0, tr("Type"));
    m_typeItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    m_propertiesTree->addTopLevelItem(m_typeItem);

    m_uidItem->setText(0, "uid");
    m_uidItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    m_propertiesTree->addTopLevelItem(m_uidItem);

    m_idItem->setText(0, "id");
    m_idItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    m_propertiesTree->addTopLevelItem(m_idItem);
    m_propertiesTree->setItemWidget(m_idItem, 1, m_idEdit);

    m_indexItem->setText(0, "index");
    m_indexItem->setData(0, Qt::DecorationRole, false); // No 'property changed' indication
    m_propertiesTree->addTopLevelItem(m_indexItem);
    m_propertiesTree->setItemWidget(m_indexItem, 1, m_indexEdit);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(2);
    layout->setContentsMargins(3, 3, 3, 3);
    layout->addWidget(m_propertiesTree);
    layout->addWidget(m_searchEdit);
}

PropertiesTree* PropertiesPane::propertiesTree() const
{
    return m_propertiesTree;
}

QSize PropertiesPane::sizeHint() const
{
    return QSize{310, 530};
}

QSpinBox* PropertiesPane::indexEdit() const
{
    return m_indexEdit;
}

QLineEdit* PropertiesPane::idEdit() const
{
    return m_idEdit;
}

QTreeWidgetItem* PropertiesPane::indexItem() const
{
    return m_indexItem;
}

QTreeWidgetItem* PropertiesPane::idItem() const
{
    return m_idItem;
}

QTreeWidgetItem* PropertiesPane::uidItem() const
{
    return m_uidItem;
}

QTreeWidgetItem* PropertiesPane::typeItem() const
{
    return m_typeItem;
}

LineEdit* PropertiesPane::searchEdit() const
{
    return m_searchEdit;
}
