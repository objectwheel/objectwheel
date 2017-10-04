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

using namespace Fit;

PropertiesWidget::PropertiesWidget(QWidget *parent)
	: QWidget(parent)
    , _color(QColor("#52616D"))
{
	setAutoFillBackground(true);
	QPalette p(palette());
    p.setColor(QPalette::Window, _color);
    setPalette(p);

    _listWidget.setStyleSheet(QString("QListView::item{background:none;border: 0px solid transparent;}"
                                        "QListView { border:0px solid white;background:rgba(%1,%2,%3,%4);}")
                                .arg(_color.red()).arg(_color.green())
                                .arg(_color.blue()).arg(_color.alpha()));
    _listWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _listWidget.setHorizontalScrollMode(ListWidget::ScrollPerPixel);
    _listWidget.setVerticalScrollMode(ListWidget::ScrollPerPixel);
    _listWidget.setSelectionBehavior(ListWidget::SelectRows);
    _listWidget.setFocusPolicy(Qt::NoFocus);
    _listWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _layout.setSpacing(fit(10));
    _layout.setContentsMargins(fit(10), fit(5), fit(5), fit(10));

    _searchEdit.setPlaceholderText("Filter");
    _searchEdit.show();
    connect(&_searchEdit, SIGNAL(textEdited(QString)), SLOT(refreshList()));

    _layout.addWidget(&_searchEdit);
    _layout.addWidget(&_listWidget);

    setLayout(&_layout);

    /* Prepare Properties Widget */
    connect(DesignManager::formScene(), SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
    connect(DesignManager::controlScene(), SIGNAL(selectionChanged()), SLOT(handleSelectionChange()));
}

const QColor& PropertiesWidget::color() const
{
    return _color;
}

void PropertiesWidget::setColor(const QColor& color)
{
    _color = color;
	QPalette p(palette());
    p.setColor(QPalette::Window, _color);
	setPalette(p);
}

void PropertiesWidget::clearList()
{
    for (int i = 0; i < _listWidget.count(); i++) {
        auto item = _listWidget.item(i);
        auto itemWidget =  _listWidget.itemWidget(item);
        if (itemWidget)
            itemWidget->deleteLater();
    }
    _listWidget.clear();
}

void PropertiesWidget::refreshList()
{
    clearList();

    auto selectedControls = DesignManager::currentScene()->selectedControls();

    if (selectedControls.size() != 1)
        return;

    auto properties = selectedControls[0]->properties();
    for (auto property : properties.keys()) {
        if (!property.contains(_searchEdit.text(), Qt::CaseInsensitive))
            continue;

        QListWidgetItem* item = new QListWidgetItem;
        PropertyItem* propertyItem = new PropertyItem(selectedControls[0], property);

        if (!propertyItem->isValid()) {
            delete item;
            propertyItem->deleteLater();
            continue;
        }

        propertyItem->resize(_listWidget.width() - fit(4), propertyItem->height());
        propertyItem->setFixedWidth(_listWidget.width() - fit(4));
        item->setSizeHint(QSize(_listWidget.width() - fit(4),propertyItem->sizeHint().height()));
        _listWidget.addItem(item);
        _listWidget.setItemWidget(item, propertyItem);
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
    return QSize(fit(190), fit(400));
}
