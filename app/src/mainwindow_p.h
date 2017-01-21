/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef MAINWINDOW_P_H
#define MAINWINDOW_P_H

#include <QVariant>
#include <QApplication>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QWidget>
#include <QQuickWidget>
#include <QQuickItem>
#include <flatbutton.h>
#include <listwidget.h>
#include <propertieswidget.h>
#include <bindingwidget.h>
#include <titlebar.h>
#include <about.h>
#include <bubblehead.h>
#include <qmleditor.h>
#include <pageswidget.h>
#include <fit.h>

using namespace Fit;

QT_BEGIN_NAMESPACE

class MainWindowPrivate
{
	public:
		QWidget* centralWidget;
		QVBoxLayout* verticalLayout;
		TitleBar* titleBar;
		QQuickWidget* designWidget;
		QHBoxLayout* buttonsLayout;
		QSpacerItem* horizontalSpacer;
		FlatButton* editButton;
		FlatButton* clearButton;
		QSpacerItem* horizontalSpacer_2;
		QWidget* toolboxWidget;
		QVBoxLayout* toolboxVLay;
		QHBoxLayout* toolboxHLay;
		ListWidget* toolboxList;
		FlatButton* toolboxAddButton;
		FlatButton* toolboxEditButton;
		FlatButton* toolboxRemoveButton;
		FlatButton* toolboxResetButton;
		PropertiesWidget* propertiesWidget;
		BindingWidget* bindingWidget;
		PagesWidget* pagesWidget;
		About* aboutWidget;
		FlatButton* aboutButton;
		BubbleHead* bubbleHead;
		QmlEditor* qmlEditor;

		void setupUi(QWidget *MainWindow)
		{
			MainWindow->setObjectName(QStringLiteral("MainWindow"));
			MainWindow->setStyleSheet(QLatin1String("#centralWidget, #MainWindow{\n"
													"background:\"#e0e4e7\";\n }"));
			centralWidget = new QWidget(MainWindow);
			centralWidget->setObjectName(QStringLiteral("centralWidget"));
			verticalLayout = new QVBoxLayout(centralWidget);
			verticalLayout->setSpacing(0);
			verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
			verticalLayout->setContentsMargins(0, 0, 0, 0);
			titleBar = new TitleBar(centralWidget);
			titleBar->setObjectName(QStringLiteral("titleBar"));
			titleBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			titleBar->setMinimumSize(QSize(0, 48));
			titleBar->setMaximumSize(QSize(16777215, 48));
			verticalLayout->addWidget(titleBar);

			designWidget = new QQuickWidget(centralWidget);
			designWidget->setObjectName(QStringLiteral("designWidget"));
			designWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			designWidget->setMouseTracking(false);
			designWidget->setSource(QUrl("qrc:/resources/qmls/design-area.qml"));
			verticalLayout->addWidget(designWidget);

			buttonsLayout = new QHBoxLayout();
			buttonsLayout->setSpacing(0);
			buttonsLayout->setObjectName(QStringLiteral("buttonsLayout"));
			buttonsLayout->setContentsMargins(0, 0, 0, 0);
			horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			buttonsLayout->addItem(horizontalSpacer);

			editButton = new FlatButton(centralWidget);
			editButton->setObjectName(QStringLiteral("editButton"));
			editButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			editButton->setMinimumSize(QSize(82, 35));
			editButton->setMaximumSize(QSize(82, 35));
			QIcon icon;
			icon.addFile(QStringLiteral(":/resources/images/edit-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
			editButton->setIcon(icon);
			editButton->setCheckable(true);
			editButton->setChecked(false);
			buttonsLayout->addWidget(editButton);

			clearButton = new FlatButton(centralWidget);
			clearButton->setObjectName(QStringLiteral("clearButton"));
			clearButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			clearButton->setMinimumSize(QSize(82, 35));
			clearButton->setMaximumSize(QSize(82, 35));
			QIcon icon1;
			icon1.addFile(QStringLiteral(":/resources/images/trash-icon.png"), QSize(), QIcon::Normal, QIcon::Off);
			clearButton->setIcon(icon1);

			buttonsLayout->addWidget(clearButton);
			horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
			buttonsLayout->addItem(horizontalSpacer_2);
			verticalLayout->addLayout(buttonsLayout);

			toolboxList = new ListWidget(centralWidget);
			toolboxList->setObjectName(QStringLiteral("toolboxList"));
			toolboxList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			toolboxList->setFocusPolicy(Qt::NoFocus);
			toolboxList->setStyleSheet(QLatin1String("QListView {\n"
													 "	border:0px solid white;\n"
													 "	background:#44504e;\n"
													 "	padding-right:5px;\n"
													 "}"
													 "QListView::item {\n"
													 "	color:white;\n"
													 "    border: 0px solid transparent;\n"
													 "	padding:2px;\n"
													 "}"
													 "QListView::item:selected {\n"
													 "	color:black;\n"
													 "  background: #e0e4e7;\n"
													 "  border: 0px solid transparent;\n"
													 "  border-radius: 3px;\n"
													 "	padding:2px;\n"
													 "  margin-right: 2px;\n"
													 "}"));
			toolboxList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			toolboxList->setDragEnabled(true);
			toolboxList->setDragDropMode(QAbstractItemView::InternalMove);
			toolboxList->setSelectionBehavior(QAbstractItemView::SelectRows);
			toolboxList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
			toolboxList->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

			toolboxWidget = new QWidget;
			toolboxVLay = new QVBoxLayout;
			toolboxHLay = new QHBoxLayout;

			toolboxAddButton = new FlatButton;
			toolboxAddButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			toolboxAddButton->setColor("#1e8145");
			toolboxAddButton->setFixedSize(fit(30),fit(30));
			toolboxAddButton->setRadius(fit(15));
			toolboxAddButton->setIconSize(QSize(fit(16),fit(16)));
			toolboxAddButton->setIcon(QIcon(":/resources/images/plus.png"));
			QObject::connect(toolboxAddButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxAddButtonClicked()) );

			toolboxRemoveButton = new FlatButton;
			toolboxRemoveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			toolboxRemoveButton->setColor("#c03638");
			toolboxRemoveButton->setFixedSize(fit(30),fit(30));
			toolboxRemoveButton->setRadius(fit(15));
			toolboxRemoveButton->setIconSize(QSize(fit(16),fit(16)));
			toolboxRemoveButton->setIcon(QIcon(":/resources/images/minus.png"));
			toolboxRemoveButton->setDisabled(true);
			QObject::connect(toolboxRemoveButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxRemoveButtonClicked()) );

			toolboxEditButton = new FlatButton;
			toolboxEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			toolboxEditButton->setColor("#2b5796");
			toolboxEditButton->setFixedSize(fit(30),fit(30));
			toolboxEditButton->setRadius(fit(15));
			toolboxEditButton->setIconSize(QSize(fit(16),fit(16)));
			toolboxEditButton->setIcon(QIcon(":/resources/images/edit.png"));
			toolboxEditButton->setDisabled(true);
			QObject::connect(toolboxEditButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxEditButtonClicked()) );

			toolboxResetButton = new FlatButton;
			toolboxResetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
			toolboxResetButton->setColor("#ee8800");
			toolboxResetButton->setFixedSize(fit(30),fit(30));
			toolboxResetButton->setRadius(fit(15));
			toolboxResetButton->setIconSize(QSize(fit(16),fit(16)));
			toolboxResetButton->setIcon(QIcon(":/resources/images/reset.png"));
			QObject::connect(toolboxResetButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxResetButtonClicked()) );

			toolboxHLay->addWidget(toolboxAddButton);
			toolboxHLay->addStretch();
			toolboxHLay->addWidget(toolboxRemoveButton);
			toolboxHLay->addStretch();
			toolboxHLay->addWidget(toolboxEditButton);
			toolboxHLay->addStretch();
			toolboxHLay->addWidget(toolboxResetButton);

			toolboxVLay->addWidget(toolboxList);
			toolboxVLay->addLayout(toolboxHLay);
			toolboxWidget->setLayout(toolboxVLay);
			toolboxWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
			toolboxHLay->setSpacing(0);
			toolboxHLay->setContentsMargins(fit(4),fit(4),fit(10),fit(8));
			toolboxVLay->setSpacing(0);
			toolboxVLay->setContentsMargins(fit(6),0,0,0);

			QObject::connect(toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){toolboxRemoveButton->setEnabled(i>=0);});
			QObject::connect(toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){toolboxEditButton->setEnabled(i>=0);});

			propertiesWidget = new PropertiesWidget(centralWidget);
			propertiesWidget->setObjectName(QStringLiteral("propertiesWidget"));
			propertiesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			bindingWidget = new BindingWidget(centralWidget);
			bindingWidget->setObjectName(QStringLiteral("bindingWidget"));
			bindingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			pagesWidget = new PagesWidget(centralWidget);
			pagesWidget->setObjectName(QStringLiteral("pagesWidget"));
			pagesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			retranslateUi(MainWindow);

			QMetaObject::connectSlotsByName(MainWindow);
		} // setupUi

		void retranslateUi(QWidget *MainWindow)
		{
			MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Objectwheel", 0));
			editButton->setText(QApplication::translate("MainWindow", "Edit", 0));
			clearButton->setText(QApplication::translate("MainWindow", "Clear", 0));
		} // retranslateUi

};

#endif // MAINWINDOW_P_H
