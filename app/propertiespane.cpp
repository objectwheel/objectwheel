#include <propertiespane.h>
#include <centralwidget.h>
#include <css.h>
#include <saveutils.h>
#include <delayer.h>
#include <filemanager.h>
#include <focuslesslineedit.h>
#include <previewresult.h>
#include <control.h>
#include <designerscene.h>
#include <controlpreviewingmanager.h>
#include <windowmanager.h>
#include <mainwindow.h>
#include <inspectorpane.h>

#include <QHeaderView>
#include <QScrollBar>

// FIXME: x, y, width, height, z properties are not shown (because we removed them within previewer)

PropertiesPane::PropertiesPane(DesignerScene* designerScene, QWidget* parent) : QTreeWidget(parent)
  , m_designerScene(designerScene)
  , m_searchEdit(new FocuslessLineEdit(this))
{
    QPalette p(palette());
    p.setColor(backgroundRole(), "#ececec");
    setAutoFillBackground(true);
    setPalette(p);

    QPalette p2(palette());
    p2.setColor(QPalette::All, QPalette::Base, QColor("#fffefc"));
    p2.setColor(QPalette::All, QPalette::Highlight, QColor("#ebd5c0"));
    p2.setColor(QPalette::All, QPalette::Text, Qt::black);
    p2.setColor(QPalette::All, QPalette::HighlightedText, Qt::black);
    setPalette(p2);

    setColumnCount(2);
    setIndentation(16);
    setDragEnabled(false);
    setUniformRowHeights(true);
    setDropIndicatorShown(false);
    setExpandsOnDoubleClick(true);
//    setItemDelegate(new PropertiesListDelegate(this));
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setSelectionBehavior(QTreeWidget::SelectRows);
    setSelectionMode(QTreeWidget::SingleSelection);
    setEditTriggers(QAbstractItemView::AllEditTriggers);
    setVerticalScrollMode(QTreeWidget::ScrollPerPixel);
    setHorizontalScrollMode(QTreeWidget::ScrollPerPixel);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont fontMedium(font());
    fontMedium.setWeight(QFont::Medium);

    header()->setFont(fontMedium);
    header()->setFixedHeight(23);
    header()->resizeSection(0, 180);
    header()->resizeSection(1, 120);
    headerItem()->setText(1, tr("Value"));
    headerItem()->setText(0, tr("Property"));
    verticalScrollBar()->setStyleSheet(CSS_SCROLLBAR);
    horizontalScrollBar()->setStyleSheet(CSS_SCROLLBAR_H);
    setStyleSheet("QTreeView {"
                  "    border: 1px solid #8c6a48;"
                  "    outline: 0;"
                  "} QHeaderView::section {"
                  "    padding-left: 5px;"
                  "    color: white;"
                  "    border: none;"
                  "    border-bottom: 1px solid #8c6a48;"
                  "    background: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.5, y2:1,"
                  "                                stop:0 #AB8157, stop:1 #9C7650);"
                  "}");

    m_searchEdit->setPlaceholderText("Filter");
    m_searchEdit->setClearButtonEnabled(true);
    m_searchEdit->setFixedHeight(22);
    connect(m_searchEdit, SIGNAL(textChanged(QString)), SLOT(filterList(QString)));

    /* Prepare Properties Widget */
    connect(m_designerScene, SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(geometryChanged(Control*)), SLOT(handleSelectionChange()));
    // BUG   connect(ControlMonitoringManager::instance(), SIGNAL(zValueChanged(Control*)), SLOT(handleSelectionChange()));
}

void PropertiesPane::sweep()
{
    // TODO
}

void PropertiesPane::updateGeometries()
{
    QTreeWidget::updateGeometries();
    QMargins vm = viewportMargins();
    vm.setBottom(m_searchEdit->height());
    QRect vg = viewport()->geometry();
    QRect geometryRect(vg.left(), vg.bottom(), vg.width(), m_searchEdit->height());
    setViewportMargins(vm);
    m_searchEdit->setGeometry(geometryRect);
}

QSize PropertiesPane::sizeHint() const
{
    return QSize{340, 700};
}

#include "propertiespane.moc"
