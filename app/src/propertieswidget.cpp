#include <propertieswidget.h>
#include <propertyitem.h>
#include <fit.h>
#include <listwidget.h>
#include <designmanager.h>

#include <QStyleOption>
#include <QPainter>
#include <QQuickItem>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QQmlContext>
#include <QIcon>
#include <QHeaderView>

using namespace Fit;

#define COLOR_BACKGROUND (QColor("#e0e4e7"))

PropertiesWidget::PropertiesWidget(QWidget *parent) : QWidget(parent)
{
	setAutoFillBackground(true);
    QPalette p(palette());
    p.setColor(QPalette::Window, COLOR_BACKGROUND);
    setPalette(p);

    QPalette p2(_treeWidget.palette());
    p2.setColor(QPalette::Base, QColor("#F3F7FA"));
    p2.setColor(QPalette::Background, COLOR_BACKGROUND);
    _treeWidget.setPalette(p2);

    _treeWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _treeWidget.setHorizontalScrollMode(ListWidget::ScrollPerPixel);
    _treeWidget.setVerticalScrollMode(ListWidget::ScrollPerPixel);
    _treeWidget.setSelectionBehavior(ListWidget::SelectRows);
    _treeWidget.setFocusPolicy(Qt::NoFocus);
    _treeWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _treeWidget.setColumnCount(2);
    _treeWidget.headerItem()->setText(0, "Property");
    _treeWidget.headerItem()->setText(1, "Value");

    _layout.setSpacing(fit(2));
    _layout.setContentsMargins(fit(3), fit(3), fit(3), fit(3));

    _searchEdit.setPlaceholderText("Filter");
    _searchEdit.show();
    connect(&_searchEdit, SIGNAL(textEdited(QString)), SLOT(refreshList()));

    _layout.addWidget(&_searchEdit);
    _layout.addWidget(&_treeWidget);

    setLayout(&_layout);

    /* Prepare Properties Widget */
    connect(DesignManager::formScene(), SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    connect(DesignManager::controlScene(), SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
}

void PropertiesWidget::clearList()
{
    for (int i = 0; i < _treeWidget.topLevelItemCount(); i++) {
        auto item = _treeWidget.topLevelItem(i);
        auto itemWidget = _treeWidget.itemWidget(item, 1);
        if (itemWidget)
            itemWidget->deleteLater();
    }
    _treeWidget.clear();
}

void PropertiesWidget::refreshList()
{
    clearList();

    auto selectedControls = DesignManager::currentScene()->selectedControls();

    if (selectedControls.size() != 1)
        return;

    auto properties = selectedControls[0]->properties();

    {
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, "Type");
        {
            QTreeWidgetItem* iitem = new QTreeWidgetItem;
            iitem->setText(0, "Type");
            iitem->setText(1, properties.first().first);
            item->addChild(iitem);

            QTreeWidgetItem* jitem = new QTreeWidgetItem;
            jitem->setText(0, "id");
            jitem->setText(1, selectedControls[0]->id());
            item->addChild(jitem);
        }
        _treeWidget.addTopLevelItem(item);
        _treeWidget.expandItem(item);
    }

    for (auto property : properties) {
        if (property.second.isEmpty())
            continue;

        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, property.first);

        for (auto propertyName : property.second.keys()) {
            //            if (superClass->property(i).isWritable() &&
            //                !QString(superClass->property(i).name()).startsWith("__") &&
            //                !QString(superClass->property(i).read(object).typeName())
            //                .contains(QRegExp("Q([A-Za-z_][A-Za-z0-9_]*)\\*")))

            QTreeWidgetItem* iitem = new QTreeWidgetItem;
            iitem->setText(0, propertyName);
            iitem->setText(1, property.second[propertyName].typeName());
            item->addChild(iitem);
        }
        _treeWidget.addTopLevelItem(item);
        _treeWidget.expandItem(item);
    }
}

void PropertiesWidget::handleSelectionChange()
{
    auto selectedControls = DesignManager::currentScene()->selectedControls();

    if (selectedControls.size() != 1) {
        setDisabled(true);
        return;
    }

    setEnabled(true);

    refreshList();
}

QSize PropertiesWidget::sizeHint() const
{
    return QSize(fit(260), fit(400));
}
