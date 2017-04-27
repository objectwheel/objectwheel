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
#include <lineedit.h>
#include <QLineEdit>
#include <filemanager.h>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <mainwindow.h>
#include <projectsscreen.h>
#include <loginscreen.h>

#define DURATION 500

using namespace Fit;

QT_BEGIN_NAMESPACE

class MainWindowPrivate
{
		typedef QList<QQuickItem*> QQuickItemList;
		typedef QList<QUrl> QQuickUrlList;

	public:
		MainWindow* parent;
		QWidget* centralWidget;
		ProjectsScreen* projectsScreen;
        LoginScreen* loginScreen;
		QVBoxLayout* verticalLayout;
		TitleBar* titleBar;
		QQuickWidget* designWidget;
		QWidget* toolboxWidget;
		QVBoxLayout* toolboxVLay;
		QWidget* toolboxAdderAreaWidget;
		QVBoxLayout* toolboxAdderAreaVLay;
		QHBoxLayout* toolboxAdderAreaButtonSideHLay;
		ListWidget* toolboxList;
		ListWidget* sceneList;
		FlatButton* toolboxAddButton;
		FlatButton* toolboxEditButton;
		FlatButton* toolboxRemoveButton;
		FlatButton* toolboxResetButton;
		FlatButton* toolboxExportButton;
		FlatButton* toolboxImportButton;
		QVBoxLayout* toolboxAdderAreaEditingLayout;
		LineEdit* toolboxUrlBox;
		LineEdit* toolBoxNameBox;
        PropertiesWidget* propertiesWidget;
		BindingWidget* bindingWidget;
		PagesWidget* pagesWidget;
		About* aboutWidget;
		FlatButton* aboutButton;
		BubbleHead* bubbleHead;
		QmlEditor* qmlEditor;
		QQuickItemList m_Items;
		QQuickUrlList m_ItemUrls;
        bool editMode;

		MainWindowPrivate(MainWindow* uparent);
		void setupUi(QWidget *MainWindow); // setupUi
		void retranslateUi(QWidget *MainWindow); // retranslateUi
		void showAdderArea();
		void hideAdderArea();
};

MainWindowPrivate::MainWindowPrivate(MainWindow* uparent)
	: parent(uparent)
{
    editMode = false;
}

void MainWindowPrivate::setupUi(QWidget* MainWindow)
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

	projectsScreen = new ProjectsScreen(MainWindow);
    loginScreen = new LoginScreen(MainWindow);

	designWidget = new QQuickWidget(centralWidget);
	designWidget->setObjectName(QStringLiteral("designWidget"));
	designWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	designWidget->setMouseTracking(false);
	designWidget->setSource(QUrl("qrc:/resources/qmls/dashboard.qml"));
	verticalLayout->addWidget(designWidget);

	toolboxList = new ListWidget(centralWidget);
	toolboxList->setIconSize(fit({30, 30}));
	toolboxList->setObjectName(QStringLiteral("toolboxList"));
	toolboxList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	toolboxList->setFocusPolicy(Qt::NoFocus);
	toolboxList->setStyleSheet(QLatin1String("QListView {\n"
											 "	border:0px solid white;\n"
											 "	background:#566573;\n"
											 "	padding-right:5px;\n"
											 "}"
											 "QListView::item {\n"
											 "	color:white;\n"
                                             "  border: 0px solid transparent;\n"
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

	sceneList = new ListWidget(centralWidget);
    sceneList->setIconSize(fit({30, 30}));
    sceneList->setObjectName(QStringLiteral("sceneList"));
    sceneList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    sceneList->setFocusPolicy(Qt::NoFocus);
    sceneList->setStyleSheet(QLatin1String("QListView {\n"
                                           "	border:0px solid white;\n"
                                           "	background:#566573;\n"
                                           "	padding-right:5px;\n"
                                           "    padding-left:10px;\n"
                                           "}"
                                           "QListView::item {\n"
                                           "	color:white;\n"
                                           "    border: 0px solid transparent;\n"
                                           "	padding:5px;\n"
                                           "}"
                                           "QListView::item:selected {\n"
                                           "	color:black;\n"
                                           "    background: #e0e4e7;\n"
                                           "    border: 0px solid transparent;\n"
                                           "    border-radius: 3px;\n"
                                           "	padding:2px;\n"
                                           "    margin-right: 2px;\n"
                                           "}"));
    sceneList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sceneList->setDragEnabled(false);
    sceneList->setDragDropMode(QAbstractItemView::NoDragDrop);
    sceneList->setSelectionBehavior(QAbstractItemView::SelectRows);
    sceneList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	sceneList->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

	toolboxWidget = new QWidget;
	toolboxVLay = new QVBoxLayout;
	toolboxAdderAreaWidget = new QWidget;
	toolboxAdderAreaVLay = new QVBoxLayout;
	toolboxAdderAreaButtonSideHLay = new QHBoxLayout;

	toolboxAddButton = new FlatButton;
	toolboxAddButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxAddButton->setColor("#1e8145");
	toolboxAddButton->setFixedSize(fit(20),fit(20));
	toolboxAddButton->setRadius(fit(4));
	toolboxAddButton->setIconSize(QSize(fit(16),fit(16)));
	toolboxAddButton->setIcon(QIcon(":/resources/images/plus.png"));
	QObject::connect(toolboxAddButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxAddButtonClicked()) );

	toolboxRemoveButton = new FlatButton;
	toolboxRemoveButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxRemoveButton->setColor("#c03638");
	toolboxRemoveButton->setFixedSize(fit(20),fit(20));
	toolboxRemoveButton->setRadius(fit(4));
	toolboxRemoveButton->setIconSize(QSize(fit(16),fit(16)));
	toolboxRemoveButton->setIcon(QIcon(":/resources/images/minus.png"));
	toolboxRemoveButton->setDisabled(true);
	QObject::connect(toolboxRemoveButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxRemoveButtonClicked()) );

	toolboxEditButton = new FlatButton;
	toolboxEditButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxEditButton->setColor("#2b5796");
	toolboxEditButton->setFixedSize(fit(20),fit(20));
	toolboxEditButton->setRadius(fit(4));
    toolboxEditButton->setCheckedColor(QColor("#2b5796").darker(    110));
	toolboxEditButton->setCheckable(true);
	toolboxEditButton->setIconSize(QSize(fit(16),fit(16)));
	toolboxEditButton->setIcon(QIcon(":/resources/images/edit.png"));
	toolboxEditButton->setDisabled(true);
	QObject::connect(toolboxEditButton, SIGNAL(toggled(bool)), MainWindow, SLOT(toolboxEditButtonToggled(bool)) );

	toolboxResetButton = new FlatButton;
	toolboxResetButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxResetButton->setColor("#ee8800");
	toolboxResetButton->setFixedSize(fit(20),fit(20));
	toolboxResetButton->setRadius(fit(4));
	toolboxResetButton->setIconSize(QSize(fit(16),fit(16)));
	toolboxResetButton->setIcon(QIcon(":/resources/images/reset.png"));
	QObject::connect(toolboxResetButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxResetButtonClicked()) );

	toolboxImportButton = new FlatButton;
	toolboxImportButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxImportButton->setColor("#1e8145");
	toolboxImportButton->setFixedSize(fit(20),fit(20));
	toolboxImportButton->setRadius(fit(4));
	toolboxImportButton->setIconSize(QSize(fit(16),fit(16)));
	toolboxImportButton->setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(-90))));
	QObject::connect(toolboxImportButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxImportButtonClicked()) );

	toolboxExportButton = new FlatButton;
	toolboxExportButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxExportButton->setColor("#c03638");
	toolboxExportButton->setFixedSize(fit(20),fit(20));
	toolboxExportButton->setRadius(fit(4));
	toolboxExportButton->setIconSize(QSize(fit(16),fit(16)));
	toolboxExportButton->setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(90))));
	toolboxExportButton->setDisabled(true);
	QObject::connect(toolboxExportButton, SIGNAL(clicked(bool)), MainWindow, SLOT(toolboxExportButtonClicked()) );

	toolboxAdderAreaButtonSideHLay->addWidget(toolboxAddButton);
	toolboxAdderAreaButtonSideHLay->addStretch();
	toolboxAdderAreaButtonSideHLay->addWidget(toolboxRemoveButton);
	toolboxAdderAreaButtonSideHLay->addStretch();
	toolboxAdderAreaButtonSideHLay->addWidget(toolboxEditButton);
	toolboxAdderAreaButtonSideHLay->addStretch();
	toolboxAdderAreaButtonSideHLay->addWidget(toolboxResetButton);
	toolboxAdderAreaButtonSideHLay->addWidget(toolboxImportButton);
	toolboxAdderAreaButtonSideHLay->addStretch();
	toolboxAdderAreaButtonSideHLay->addWidget(toolboxExportButton);

	toolboxUrlBox = new LineEdit;
	toolboxUrlBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	toolboxUrlBox->setFixedHeight(fit(30));
	toolboxUrlBox->setIcon(QIcon(":/resources/images/web.png"));
	toolboxUrlBox->setPlaceholderText("Icon url");
	toolboxUrlBox->setText(":/resources/images/item.png");
	toolboxUrlBox->setDisabled(true);
	toolboxUrlBox->setHidden(true);
	QObject::connect(toolboxUrlBox->lineEdit(), SIGNAL(textChanged(QString)),
					 MainWindow, SLOT(handleToolboxUrlboxChanges(QString)));

	toolBoxNameBox = new LineEdit;
	toolBoxNameBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	toolBoxNameBox->setFixedHeight(fit(30));
	toolBoxNameBox->setIcon(QIcon(":/resources/images/item.png"));
	toolBoxNameBox->setPlaceholderText("Tool name");
	toolBoxNameBox->setDisabled(true);
	toolBoxNameBox->setHidden(true);
	QObject::connect(toolBoxNameBox->lineEdit(), SIGNAL(textChanged(QString)),
					 MainWindow, SLOT(handleToolboxNameboxChanges(QString)));

    toolboxAdderAreaEditingLayout = new QVBoxLayout;
	toolboxAdderAreaEditingLayout->addWidget(toolBoxNameBox);
	toolboxAdderAreaEditingLayout->addWidget(toolboxUrlBox);
    toolboxAdderAreaEditingLayout->setSpacing(0);
	toolboxAdderAreaEditingLayout->setContentsMargins(0,0,0,0);

	toolboxAdderAreaButtonSideHLay->setSpacing(fit(6));
	toolboxAdderAreaButtonSideHLay->setContentsMargins(0,0,0,0);
	toolboxAdderAreaVLay->addLayout(toolboxAdderAreaButtonSideHLay);
	toolboxAdderAreaVLay->addLayout(toolboxAdderAreaEditingLayout);
	toolboxAdderAreaVLay->addStretch();
	toolboxAdderAreaVLay->setSpacing(fit(6));
	toolboxAdderAreaVLay->setContentsMargins(fit(6),fit(6),fit(6),fit(6));
	toolboxAdderAreaWidget->setLayout(toolboxAdderAreaVLay);
	toolboxAdderAreaWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	toolboxAdderAreaWidget->setFixedSize(fit(155), fit(32));
	toolboxAdderAreaWidget->setObjectName("toolboxAdderAreaWidget");
	toolboxAdderAreaWidget->setStyleSheet(QString("#toolboxAdderAreaWidget{border:none; border-radius:%1;}").arg(fit(5)));

	toolboxVLay->addWidget(toolboxList);
	toolboxVLay->addWidget(toolboxAdderAreaWidget);
	toolboxVLay->setSpacing(0);
	toolboxVLay->setContentsMargins(fit(6),0,0,0);
	toolboxWidget->setLayout(toolboxVLay);
	toolboxWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QObject::connect(toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
		if (i>=0) {
			toolboxUrlBox->setText(dname(toolboxList->GetUrls(toolboxList->currentItem())[0].toLocalFile()) + "/icon.png");
            toolBoxNameBox->setText(toolboxList->currentItem()->text());
		}
		toolBoxNameBox->setEnabled(i>=0);
		toolboxUrlBox->setEnabled(i>=0);
        if (!toolboxEditButton->isChecked()) {
			toolboxEditButton->setEnabled(i>=0);
			toolboxRemoveButton->setEnabled(i>=0);
			toolboxExportButton->setEnabled(i>=0);
		}
	});

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
}

void MainWindowPrivate::retranslateUi(QWidget* MainWindow)
{
	MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Objectwheel", 0));
}

void MainWindowPrivate::showAdderArea()
{
	QPropertyAnimation *animation = new QPropertyAnimation(toolboxAdderAreaWidget, "minimumHeight");
	animation->setDuration(DURATION);
	animation->setStartValue(fit(32));
    animation->setEndValue(fit(    110));
	animation->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

	QPropertyAnimation *animation2 = new QPropertyAnimation(toolboxAdderAreaWidget, "maximumHeight");
	animation2->setDuration(DURATION);
	animation2->setStartValue(fit(32));
    animation2->setEndValue(fit(    110));
	animation2->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

	QParallelAnimationGroup *group = new QParallelAnimationGroup;
	group->addAnimation(animation);
	group->addAnimation(animation2);
	QObject::connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
	group->start();

	toolboxAdderAreaEditingLayout->setSpacing(fit(6));
	toolboxAdderAreaEditingLayout->setContentsMargins(0,0,0,0);
    toolBoxNameBox->setHidden(false);
	toolboxUrlBox->setHidden(false);
	toolboxAddButton->setDisabled(true);
	toolboxRemoveButton->setDisabled(true);
	toolboxImportButton->setDisabled(true);
	toolboxExportButton->setDisabled(true);
	toolboxResetButton->setDisabled(true);
}

void MainWindowPrivate::hideAdderArea()
{
	QPropertyAnimation *animation = new QPropertyAnimation(toolboxAdderAreaWidget, "minimumHeight");
	animation->setDuration(DURATION);
    animation->setStartValue(fit(    110));
	animation->setEndValue(fit(32));
	animation->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

	QPropertyAnimation *animation2 = new QPropertyAnimation(toolboxAdderAreaWidget, "maximumHeight");
	animation2->setDuration(DURATION);
    animation2->setStartValue(fit(    110));
	animation2->setEndValue(fit(32));
	animation2->setEasingCurve(QEasingCurve::OutExpo);
	QObject::connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

	QParallelAnimationGroup *group = new QParallelAnimationGroup;
	group->addAnimation(animation);
	group->addAnimation(animation2);
	QObject::connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
	group->start();

	toolboxAdderAreaEditingLayout->setSpacing(0);
	toolboxAdderAreaEditingLayout->setContentsMargins(0,0,0,0);
	toolboxAddButton->setEnabled(true);
	toolboxRemoveButton->setEnabled(true);
	toolboxImportButton->setEnabled(true);
	toolboxExportButton->setEnabled(true);
	toolboxResetButton->setEnabled(true);
	toolboxEditButton->setChecked(false);
    toolBoxNameBox->setHidden(true);
	toolboxUrlBox->setHidden(true);
}
#endif // MAINWINDOW_P_H
