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
#include <QTimer>

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
		ListWidget* toolboxWidget;
		PropertiesWidget* propertiesWidget;
		BindingWidget* bindingWidget;

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

			toolboxWidget = new ListWidget(centralWidget);
			toolboxWidget->setObjectName(QStringLiteral("toolboxWidget"));
			toolboxWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
			toolboxWidget->setFocusPolicy(Qt::NoFocus);
			toolboxWidget->setStyleSheet(QLatin1String("QListView {\n"
													   "	border:0px solid white;\n"
													   "	background:#566573;\n"
													   "	padding-right:5px;\n"
													   "}\n"
													   "\n"
													   "QListView::item {\n"
													   "	color:white;\n"
													   "    border: 0px solid transparent;\n"
													   "	padding:2px;\n"
													   "}"));
			toolboxWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			toolboxWidget->setDragEnabled(true);
			toolboxWidget->setDragDropMode(QAbstractItemView::DragOnly);
			toolboxWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
			toolboxWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
			toolboxWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

			propertiesWidget = new PropertiesWidget(centralWidget);
			propertiesWidget->setObjectName(QStringLiteral("propertiesWidget"));
			propertiesWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			bindingWidget = new BindingWidget(centralWidget);
			bindingWidget->setObjectName(QStringLiteral("bindingWidget"));
			bindingWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

			retranslateUi(MainWindow);

			QMetaObject::connectSlotsByName(MainWindow);
		} // setupUi

		void retranslateUi(QWidget *MainWindow)
		{
			MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Form", 0));
			editButton->setText(QApplication::translate("MainWindow", "Edit", 0));
			clearButton->setText(QApplication::translate("MainWindow", "Clear", 0));
		} // retranslateUi

};

#endif // MAINWINDOW_P_H
