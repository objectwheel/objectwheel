#include <fit.h>
#include <zipper.h>
#include <string.h>
#include <titlebar.h>
#include <listwidget.h>
#include <mainwindow.h>
#include <css.h>
#include <splashscreen.h>
#include <filemanager.h>
#include <projectmanager.h>
#include <usermanager.h>
#include <toolsmanager.h>
#include <savemanager.h>
#include <splashscreen.h>
#include <scenemanager.h>
#include <delayer.h>
#include <control.h>
#include <formscene.h>
#include <qmlpreviewer.h>
#include <loadingindicator.h>

#include <QtConcurrent>
#include <QtNetwork>

#define DURATION 500

using namespace Fit;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    setupGui();
    QTimer::singleShot(300, [=] { setupManagers(); });
}

void MainWindow::setupGui()
{
    setWindowTitle(QApplication::translate("MainWindow", "Objectwheel", 0));
    setObjectName(QStringLiteral("MainWindow"));
    setStyleSheet(QLatin1String("#_centralWidget, #MainWindow{\n"
                                "background:\"#e0e4e7\";\n }"));

    _centralWidget.setObjectName(QStringLiteral("_centralWidget"));
    setCentralWidget(&_centralWidget);
    verticalLayout.setSpacing(0);
    verticalLayout.setObjectName(QStringLiteral("verticalLayout"));
    verticalLayout.setContentsMargins(0, 0, 0, 0);
    _centralWidget.setLayout(&verticalLayout);

    titleBar.setObjectName(QStringLiteral("titleBar"));
    titleBar.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    titleBar.setMinimumSize(QSize(0, 45));
    titleBar.setMaximumSize(QSize(16777215, 45));
    verticalLayout.addWidget(&titleBar);

    settleWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    verticalLayout.addWidget(&settleWidget);

    designManager.setSettleWidget(&settleWidget);

    toolboxList.setIconSize(fit({30, 30}));
    toolboxList.setObjectName(QStringLiteral("toolboxList"));
    toolboxList.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolboxList.setFocusPolicy(Qt::NoFocus);
    toolboxList.setStyleSheet(QString("QListView {\n"
                                       "	border:0px solid white;\n"
                                       "	background:#52616D;\n"
                                       "	padding-right:%1px;\n"
                                       "}"
                                       "QListView::item {\n"
                                       "	color:white;\n"
                                       "  border: 0px solid transparent;\n"
                                       "	padding:%2px;\n"
                                       "}"
                                       "QListView::item:selected {\n"
                                       "	color:black;\n"
                                       "  background: #e0e4e7;\n"
                                       "  border: 0px solid transparent;\n"
                                       "  border-radius: %3px;\n"
                                       "	padding:%4px;\n"
                                       "  margin-right: %4px;\n"
                                       "}").arg(5).arg(2).arg(3).arg(2));
    toolboxList.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    toolboxList.setDragEnabled(true);
    toolboxList.setDragDropMode(QAbstractItemView::InternalMove);
    toolboxList.setSelectionBehavior(QAbstractItemView::SelectRows);
    toolboxList.setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    toolboxList.setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    toolboxAddButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolboxAddButton.setColor("#6BB64B");
    toolboxAddButton.setFixedSize(fit(20),fit(20));
    toolboxAddButton.setRadius(fit(4));
    toolboxAddButton.setIconSize(QSize(fit(15),fit(15)));
    toolboxAddButton.setIcon(QIcon(":/resources/images/plus.png"));
    connect(&toolboxAddButton, SIGNAL(clicked(bool)), SLOT(toolboxAddButtonClicked()) );

    toolboxRemoveButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolboxRemoveButton.setColor("#C61717");
    toolboxRemoveButton.setFixedSize(fit(20),fit(20));
    toolboxRemoveButton.setRadius(fit(4));
    toolboxRemoveButton.setIconSize(QSize(fit(15),fit(15)));
    toolboxRemoveButton.setIcon(QIcon(":/resources/images/minus.png"));
    toolboxRemoveButton.setDisabled(true);
    connect(&toolboxRemoveButton, SIGNAL(clicked(bool)), SLOT(toolboxRemoveButtonClicked()) );

    toolboxEditButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolboxEditButton.setColor("#0D74C8");
    toolboxEditButton.setFixedSize(fit(20),fit(20));
    toolboxEditButton.setRadius(fit(4));
    toolboxEditButton.setCheckedColor(QColor("#0D74C8").darker(    110));
    toolboxEditButton.setCheckable(true);
    toolboxEditButton.setIconSize(QSize(fit(13),fit(13)));
    toolboxEditButton.setIcon(QIcon(":/resources/images/edit.png"));
    toolboxEditButton.setDisabled(true);
    connect(&toolboxEditButton, SIGNAL(toggled(bool)), SLOT(toolboxEditButtonToggled(bool)) );

    toolboxImportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolboxImportButton.setColor("#6BB64B");
    toolboxImportButton.setFixedSize(fit(20),fit(20));
    toolboxImportButton.setRadius(fit(4));
    toolboxImportButton.setIconSize(QSize(fit(15),fit(15)));
    toolboxImportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(-90))));
    connect(&toolboxImportButton, SIGNAL(clicked(bool)), SLOT(toolboxImportButtonClicked()) );

    toolboxExportButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolboxExportButton.setColor("#C61717");
    toolboxExportButton.setFixedSize(fit(20),fit(20));
    toolboxExportButton.setRadius(fit(4));
    toolboxExportButton.setIconSize(QSize(fit(15),fit(15)));
    toolboxExportButton.setIcon(QIcon(QPixmap(":/resources/images/left-arrow.png").transformed(QTransform().rotate(90))));
    toolboxExportButton.setDisabled(true);
    connect(&toolboxExportButton, SIGNAL(clicked(bool)), SLOT(toolboxExportButtonClicked()) );

    toolboxAdderAreaButtonSideHLay.addWidget(&toolboxAddButton);
    toolboxAdderAreaButtonSideHLay.addStretch();
    toolboxAdderAreaButtonSideHLay.addWidget(&toolboxRemoveButton);
    toolboxAdderAreaButtonSideHLay.addStretch();
    toolboxAdderAreaButtonSideHLay.addWidget(&toolboxEditButton);
    toolboxAdderAreaButtonSideHLay.addStretch();
    toolboxAdderAreaButtonSideHLay.addWidget(&toolboxImportButton);
    toolboxAdderAreaButtonSideHLay.addStretch();
    toolboxAdderAreaButtonSideHLay.addWidget(&toolboxExportButton);

    toolboxUrlBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toolboxUrlBox.setFixedHeight(fit(30));
    toolboxUrlBox.setIcon(QIcon(":/resources/images/web.png"));
    toolboxUrlBox.setPlaceholderText("Icon url");
    toolboxUrlBox.setText(":/resources/images/item.png");
    toolboxUrlBox.setDisabled(true);
    toolboxUrlBox.setHidden(true);
    connect(toolboxUrlBox.lineEdit(), SIGNAL(textChanged(QString)),
            SLOT(handleToolboxUrlboxChanges(QString)));

    toolBoxNameBox.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    toolBoxNameBox.setFixedHeight(fit(30));
    toolBoxNameBox.setIcon(QIcon(":/resources/images/item.png"));
    toolBoxNameBox.setPlaceholderText("Tool name");
    toolBoxNameBox.setDisabled(true);
    toolBoxNameBox.setHidden(true);
    connect(toolBoxNameBox.lineEdit(), SIGNAL(textChanged(QString)),
            SLOT(handleToolboxNameboxChanges(QString)));

    toolboxAdderAreaEditingLayout.addWidget(&toolBoxNameBox);
    toolboxAdderAreaEditingLayout.addWidget(&toolboxUrlBox);
    toolboxAdderAreaEditingLayout.setSpacing(0);
    toolboxAdderAreaEditingLayout.setContentsMargins(0,0,0,0);

    toolboxAdderAreaButtonSideHLay.setSpacing(fit(6));
    toolboxAdderAreaButtonSideHLay.setContentsMargins(0,0,0,0);
    toolboxAdderAreaVLay.addLayout(&toolboxAdderAreaButtonSideHLay);
    toolboxAdderAreaVLay.addLayout(&toolboxAdderAreaEditingLayout);
    toolboxAdderAreaVLay.addStretch();
    toolboxAdderAreaVLay.setSpacing(fit(6));
    toolboxAdderAreaVLay.setContentsMargins(fit(6),fit(6),fit(6),fit(6));
    toolboxAdderAreaWidget.setLayout(&toolboxAdderAreaVLay);
    toolboxAdderAreaWidget.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    toolboxAdderAreaWidget.setFixedSize(fit(155), fit(32));
    toolboxAdderAreaWidget.setObjectName("toolboxAdderAreaWidget");
    toolboxAdderAreaWidget.setStyleSheet(QString("#toolboxAdderAreaWidget{border:none; border-radius:%1;}").arg(fit(5)));

    toolboxVLay.addWidget(&toolboxList);
    toolboxVLay.addWidget(&toolboxAdderAreaWidget);
    toolboxVLay.setSpacing(0);
    toolboxVLay.setContentsMargins(fit(6),0,0,0);
    toolboxWidget.setLayout(&toolboxVLay);
    toolboxWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    connect(&toolboxList, (void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
            toolboxUrlBox.setText(dname(toolboxList.GetUrls(toolboxList.currentItem())[0].toLocalFile()) + "/icon.png");
            toolBoxNameBox.setText(toolboxList.currentItem()->text());
        }
        toolBoxNameBox.setEnabled(i>=0);
        toolboxUrlBox.setEnabled(i>=0);
        if (!toolboxEditButton.isChecked()) {
            toolboxEditButton.setEnabled(i>=0);
            toolboxRemoveButton.setEnabled(i>=0);
            toolboxExportButton.setEnabled(i>=0);
        }
    });

    propertiesWidget.setObjectName(QStringLiteral("propertiesWidget"));
    propertiesWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    formsWidget.setObjectName(QStringLiteral("formsWidget"));
    formsWidget.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    formsWidget.setParent(this);

    /* Add Title Bar */
    fit(&titleBar, Fit::Height, true);
    titleBar.setText("Objectwheel Studio");
    titleBar.setColor("#0D74C8");
    titleBar.setShadowColor("#EAEEF1");

    connect(&toolboxList,(void(ListWidget::*)(int))(&ListWidget::currentRowChanged),[=](int i){
        if (i>=0) {
            DesignManager::formScene()->clearSelection();
            DesignManager::controlScene()->clearSelection();
        }
    });

    toolboxList.indicatorButton()->setIcon(QIcon(":/resources/images/right-arrow.png"));
    toolboxList.indicatorButton()->setColor(QColor("#0D74C8"));
    toolboxList.indicatorButton()->setRadius(fit(7));
    toolboxList.indicatorButton()->setIconSize(QSize(fit(10), fit(10)));
    toolboxList.indicatorButton()->resize(fit(15), fit(15));
    connect(toolboxList.indicatorButton(), &FlatButton::clicked, [=] {
        auto previousControl = DesignManager::controlScene()->mainControl();
        if (previousControl)
            previousControl->deleteLater();
        auto url = toolboxList.GetUrls(toolboxList.currentItem())[0];
        auto control = SaveManager::exposeControl(dname(dname(url.toLocalFile())));
        DesignManager::controlScene()->setMainControl(control);
        DesignManager::setMode(DesignManager::ControlGUI);
        control->refresh();
        connect(control, &Control::initialized, [=] {
            control->controlTransaction()->setTransactionsEnabled(true);
        });
        for (auto childControl : control->childControls())
            childControl->refresh();
    });

    _toolboxDockwidget.setWidget(&toolboxWidget);
    _toolboxDockwidget.setFloating(true);
    addDockWidget(Qt::RightDockWidgetArea, &_toolboxDockwidget);

    // Init Splash Screen
    SplashScreen::init(this);
    SplashScreen::setText("Loading");
    SplashScreen::setTextColor("#2e3a41");
    SplashScreen::setBackgroundBrush(QColor("#e0e4e7"));
    SplashScreen::setIcon(QIcon(":/resources/images/logo.png"));
    SplashScreen::setIconSize(Fit::fit(160), Fit::fit(80));
    SplashScreen::setLoadingSize(Fit::fit(24), Fit::fit(24));
    SplashScreen::setLoadingImageFilename("qrc:///resources/images/loading.png");
    SplashScreen::show(false);
}

void MainWindow::showAdderArea()
{
    QPropertyAnimation *animation = new QPropertyAnimation(&toolboxAdderAreaWidget, "minimumHeight");
    animation->setDuration(DURATION);
    animation->setStartValue(fit(32));
    animation->setEndValue(fit(    110));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    QPropertyAnimation *animation2 = new QPropertyAnimation(&toolboxAdderAreaWidget, "maximumHeight");
    animation2->setDuration(DURATION);
    animation2->setStartValue(fit(32));
    animation2->setEndValue(fit(    110));
    animation2->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation2);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();

    toolboxAdderAreaEditingLayout.setSpacing(fit(6));
    toolboxAdderAreaEditingLayout.setContentsMargins(0,0,0,0);
    toolBoxNameBox.setHidden(false);
    toolboxUrlBox.setHidden(false);
    toolboxAddButton.setDisabled(true);
    toolboxRemoveButton.setDisabled(true);
    toolboxImportButton.setDisabled(true);
    toolboxExportButton.setDisabled(true);
}
void MainWindow::hideAdderArea()
{
    QPropertyAnimation *animation = new QPropertyAnimation(&toolboxAdderAreaWidget, "minimumHeight");
    animation->setDuration(DURATION);
    animation->setStartValue(fit(    110));
    animation->setEndValue(fit(32));
    animation->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation, SIGNAL(finished()), animation, SLOT(deleteLater()));

    QPropertyAnimation *animation2 = new QPropertyAnimation(&toolboxAdderAreaWidget, "maximumHeight");
    animation2->setDuration(DURATION);
    animation2->setStartValue(fit(    110));
    animation2->setEndValue(fit(32));
    animation2->setEasingCurve(QEasingCurve::OutExpo);
    connect(animation2, SIGNAL(finished()), animation2, SLOT(deleteLater()));

    QParallelAnimationGroup *group = new QParallelAnimationGroup(this);
    group->addAnimation(animation);
    group->addAnimation(animation2);
    connect(group, SIGNAL(finished()), group, SLOT(deleteLater()));
    group->start();

    toolboxAdderAreaEditingLayout.setSpacing(0);
    toolboxAdderAreaEditingLayout.setContentsMargins(0,0,0,0);
    toolboxAddButton.setEnabled(true);
    toolboxRemoveButton.setEnabled(true);
    toolboxImportButton.setEnabled(true);
    toolboxExportButton.setEnabled(true);
    toolboxEditButton.setChecked(false);
    toolBoxNameBox.setHidden(true);
    toolboxUrlBox.setHidden(true);
}
void MainWindow::setupManagers()
{
    ToolsManager::setListWidget(&toolboxList);
    auto userManager = new UserManager(this);
    Q_UNUSED(userManager);
    auto* projectManager = new ProjectManager(this);
    projectManager->setMainWindow(this);
    new SaveManager(this);
    new QmlPreviewer(this);
    auto sceneManager = new SceneManager;
    sceneManager->setMainWindow(this);
//    sceneManager->setSceneListWidget(&sceneList);
    sceneManager->addScene("studioScene", &_centralWidget);
    sceneManager->addScene("projectsScene", &_projectsScreen);
    sceneManager->addScene("loginScene", &_loginScreen);
    sceneManager->addScene("aboutScene", &aboutWidget);
    sceneManager->addScene("buildsScene", &buildsScreen);
    sceneManager->setCurrent("loginScene", false);

    connect(SaveManager::instance(), SIGNAL(parserRunningChanged(bool)), SLOT(handleIndicatorChanges()));
    connect(QmlPreviewer::instance(), SIGNAL(workingChanged(bool)), SLOT(handleIndicatorChanges()));

    SplashScreen::raise();

    for (auto scene : sceneManager->scenes()) {
        QList<QUrl> urls;
        QString sceneName;
        QString iconPath = ":/resources/images/" + scene + "Icon.png";
        if (scene == "projectsScene") {
            sceneName = "Projects";
        } else if (scene == "studioScene") {
            sceneName = "Studio";
        } else if (scene == "aboutScene") {
            sceneName = "About";
        } else if (scene == "loginScene") {
            continue;
        } else if (scene == "buildsScene") {
            continue;
        }
        urls << scene;
//        QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), sceneName);
//        sceneList.insertItem(0, item);
//        sceneList.AddUrls(item, urls);
    }
    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanupObjectwheel()));

    auto ret = QtConcurrent::run(&UserManager::tryAutoLogin);
    Delayer::delay(&ret, &QFuture<bool>::isRunning);
    if (ret.result()) {
        ProjectsScreen::refreshProjectList();
        SplashScreen::hide();
        SceneManager::show("projectsScene", SceneManager::ToLeft);
    } else {
        SplashScreen::hide();
    }
}

void MainWindow::handleIndicatorChanges()
{
    DesignManager::loadingIndicator()->setRunning(SaveManager::parserWorking() || QmlPreviewer::working());
}
void MainWindow::cleanupObjectwheel()
{
    while(SaveManager::parserWorking())
        Delayer::delay(100);

    UserManager::stopUserSession();

    qApp->processEvents();
}
void MainWindow::clearStudio()
{

}
void MainWindow::on_secureExitButton_clicked()
{
    SplashScreen::setText("Stopping user session");
    SplashScreen::show(true);
    UserManager::clearAutoLogin();
    auto ret = QtConcurrent::run(&UserManager::stopUserSession);
    Delayer::delay(&ret, &QFuture<void>::isRunning);
    SplashScreen::hide();
    SplashScreen::setText("Loading");
    SceneManager::show("loginScene", SceneManager::ToLeft);
}
void MainWindow::handleToolboxUrlboxChanges(const QString& text)
{
    QPixmap pixmap;
    pixmap.loadFromData(dlfile(text));
    if (pixmap.isNull()) return;
    auto icon = dname(toolboxList.GetUrls(toolboxList.currentItem())[0].toLocalFile()) + "/icon.png";
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    if (!pixmap.save(&buffer,"PNG")) return;
    buffer.close();
    if (!wrfile(icon, bArray)) return;
    toolboxList.currentItem()->setIcon(QIcon(icon));
}
void MainWindow::handleToolboxNameboxChanges(QString /*name*/)
{
    //	if (name == toolboxList.currentItem()->text() || name == "") return;

    //	int count = 1;
    //	for (int i = 0; i < toolboxList.count(); i++) {
    //		if (toolboxList.item(i)->text() == name) {
    //			if (count > 1) {
    //				name.remove(name.size() - 1, 1);
    //			}
    //			i = -1;
    //			count++;
    //			name += QString::number(count);
    //		}
    //	}

    //	auto from = ToolsManager::toolsDir() + "/" + toolboxList.currentItem()->text();
    //	auto to = ToolsManager::toolsDir() + "/" + name;
    //	if (!rn(from, to)) qFatal("MainWindow : Error occurred");

    //	toolboxList.currentItem()->setText(name);

    //	QList<QUrl> urls;
    //	urls << QUrl::fromLocalFile(to + "/main.qml");
    //	toolboxList.RemoveUrls(toolboxList.currentItem());
    //	toolboxList.AddUrls(toolboxList.currentItem(),urls);

    //	for (int i = 0; i < m_ItemUrls.size(); i++) {
    //		if (m_ItemUrls[i].toLocalFile() == (from+"/main.qml")) {
    //			m_ItemUrls[i] = QUrl::fromLocalFile(to+"/main.qml");
    //		}
    //	}

    //	qmlEditor->updateCacheForRenamedEntry(from, to, true);
}
void MainWindow::toolboxEditButtonToggled(bool checked)
{
    if (checked)
        showAdderArea();
    else
        hideAdderArea();
}
void MainWindow::toolboxRemoveButtonClicked()
{
    //	if (toolboxList.currentRow() < 0) return;
    //	auto name = toolboxList.currentItem()->text();
    //	QMessageBox msgBox;
    //	msgBox.setText(QString("<b>This will remove %1 from Tool Library and Dashboard.</b>").arg(name));
    //	msgBox.setInformativeText("Do you want to continue?");
    //	msgBox.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    //	msgBox.setDefaultButton(QMessageBox::No);
    //	msgBox.setIcon(QMessageBox::Warning);
    //	const int ret = msgBox.exec();
    //	switch (ret) {
    //		case QMessageBox::Yes: {
    //			qmlEditor->clearCacheFor(ToolsManager::toolsDir() + separator() + name, true);
    //			rm(ToolsManager::toolsDir() + separator() + name);
    //			toolboxList.RemoveUrls(toolboxList.currentItem());
    //			delete toolboxList.takeItem(toolboxList.currentRow());

    //			for (int i = 0; i < m_ItemUrls.size(); i++) {
    //				if (m_ItemUrls[i].toLocalFile() == (ToolsManager::toolsDir() + separator() + name + "/main.qml")) {
    //					auto items = GetAllChildren(m_Items[i]);
    //					for (auto item : items) {
    //						if (m_Items.contains(item)) {
    //							SaveManager::removeSave(designWidget.rootContext()->nameForObject(item));
    //							SaveManager::removeParentalRelationship(designWidget.rootContext()->nameForObject(item));
    //							designWidget.rootContext()->setContextProperty(
    //										designWidget.rootContext()->nameForObject(item), 0);
    //							int j = m_Items.indexOf(item);
    //							m_Items.removeOne(item);
    //							m_ItemUrls.removeAt(j);
    //							item->deleteLater();
    //						}
    //					}
    //				}
    //			}
    //			HideSelectionTools();
    //			break;
    //		} default: {
    //			// Do nothing
    //			break;
    //		}
    //	}
}
void MainWindow::toolboxAddButtonClicked()
{
    int count = 1;
    auto name = QString("Item%1").arg(count);
    for (int i = 0; i < toolboxList.count(); i++) {
        if (toolboxList.item(i)->text() == name) {
            name.remove(name.size() - 1, 1);
            i = -1;
            count++;
            name += QString::number(count);
        }
    }

    auto itemPath = ToolsManager::toolsDir() + separator() + name;
    auto iconPath = itemPath + separator() + DIR_THIS + separator() + "icon.png";
    auto qmlPath = itemPath + separator() + DIR_THIS + separator() + "main.png";

    if (!mkdir(itemPath) || !cp(DIR_QRC_ITEM, itemPath, true, true))
        return;

    SaveManager::refreshToolUid(itemPath);

    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(qmlPath);

    QListWidgetItem* item = new QListWidgetItem(QIcon(iconPath), name);
    toolboxList.insertItem(0, item);
    toolboxList.AddUrls(item, urls);
    toolboxList.setCurrentRow(0);
    toolboxEditButton.setChecked(true);
}
void MainWindow::toolboxExportButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::Directory);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setOption(QFileDialog::ShowDirsOnly, true);
    if (dialog.exec()) {
        auto dir = dname(dname(toolboxList.GetUrls(toolboxList.currentItem())[0].toLocalFile()));
        auto toolName = toolboxList.currentItem()->text();
        if (!rm(dialog.selectedFiles().at(0) + separator() + toolName + ".zip")) return;
        Zipper::compressDir(dir, dialog.selectedFiles().at(0) + separator() + toolName + ".zip");
        QMessageBox::information(this, "Done", "Tool export is done.");
    }
}
void MainWindow::toolboxImportButtonClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Zip files (*.zip)"));
    dialog.setViewMode(QFileDialog::Detail);
    if (dialog.exec()) {
        handleImports(dialog.selectedFiles());
        QMessageBox::information(this, "Done", "Tool import is done.");
    }
}
void MainWindow::handleImports(const QStringList& fileNames)
{
    for (auto fileName : fileNames) {
        auto name = fname(fileName.remove(fileName.size() - 4, 4));
        int count = 1;
        for (int i = 0; i < toolboxList.count(); i++) {
            if (toolboxList.item(i)->text() == name) {
                if (count > 1) {
                    name.remove(name.size() - 1, 1);
                }
                i = -1;
                count++;
                name += QString::number(count);
            }
        }

        auto itemPath = ToolsManager::toolsDir() + separator() + name;
        if (!mkdir(itemPath) || !Zipper::extractZip(rdfile(fileName + ".zip"), itemPath))
            return;

        if (!SaveManager::isOwctrl(itemPath)) {
            rm(itemPath);
            return;
        }

        SaveManager::refreshToolUid(itemPath);
        ToolsManager::addTool(name);
    }
}
