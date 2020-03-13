#include <buildscontroller.h>
#include <buildspane.h>
#include <platform.h>
#include <buttonslice.h>
#include <stackedlayout.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <androidplatformwidget.h>
#include <androidplatformcontroller.h>
#include <downloadwidget.h>
#include <downloadcontroller.h>
#include <buildmodel.h>
#include <servermanager.h>

#include <QPushButton>
#include <QLabel>
#include <QListWidget>

BuildsController::BuildsController(BuildsPane* buildsPane, QObject* parent) : QObject(parent)
  , m_buildsPane(buildsPane)
  , m_downloadController(new DownloadController(m_buildsPane->downloadWidget(), this))
  , m_androidPlatformController(new AndroidPlatformController(m_buildsPane->androidPlatformWidget(), this))
{
    connect(m_buildsPane->downloadWidget()->buttonSlice()->get(DownloadWidget::New), &QPushButton::clicked,
            this, &BuildsController::onNewButtonClick);
    connect(m_buildsPane->platformSelectionWidget()->buttonSlice()->get(PlatformSelectionWidget::Back), &QPushButton::clicked,
            this, &BuildsController::onBackButtonClick);
    connect(m_buildsPane->platformSelectionWidget()->buttonSlice()->get(PlatformSelectionWidget::Next), &QPushButton::clicked,
            this, &BuildsController::onNextButtonClick);
    connect(m_buildsPane->platformSelectionWidget()->platformList(), &QListWidget::itemDoubleClicked,
            this, &BuildsController::onNextButtonClick);
    connect(m_buildsPane->androidPlatformWidget()->buttonSlice()->get(AndroidPlatformWidget::Back), &QPushButton::clicked,
            this, &BuildsController::onBackButtonClick);
    connect(m_buildsPane->androidPlatformWidget()->buttonSlice()->get(AndroidPlatformWidget::Build), &QPushButton::clicked,
            this, &BuildsController::onAndroidBuildButtonClick);
    connect(m_buildsPane->androidPlatformWidget()->buttonSlice()->get(AndroidPlatformWidget::Reset), &QPushButton::clicked,
            this, &BuildsController::onAndroidResetButtonClick);
    connect(static_cast<BuildModel*>(m_buildsPane->downloadWidget()->downloadList()->model()), &BuildModel::uploadFinished,
            this, [=] { m_buildsPane->downloadWidget()->buttonSlice()->get(DownloadWidget::New)->setEnabled(true); });
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &BuildsController::onServerDisconnect);
}

void BuildsController::charge()
{
    m_androidPlatformController->charge();
    m_buildsPane->platformSelectionWidget()->platformList()->setCurrentRow(0);
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->downloadWidget());
    static_cast<BuildModel*>(m_buildsPane->downloadWidget()->downloadList()->model())->clear();
}

void BuildsController::onNewButtonClick()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->platformSelectionWidget());
}

void BuildsController::onBackButtonClick()
{
    int nextIndex = m_buildsPane->stackedLayout()->currentIndex() - 1;
    if (nextIndex < 0)
        return;
    m_buildsPane->stackedLayout()->setCurrentIndex(nextIndex);
}

void BuildsController::onNextButtonClick()
{
    if (auto nextWidget = widgetForPlatform(m_buildsPane->platformSelectionWidget()->currentPlatform())) {
        m_buildsPane->stackedLayout()->setCurrentWidget(nextWidget);
    } else {
        UtilityFunctions::showMessage(m_buildsPane,
                                      tr("No platform selected"),
                                      tr("Please select a platform to continue."),
                                      QMessageBox::Warning);
    }
}

void BuildsController::onAndroidBuildButtonClick()
{
    if (m_androidPlatformController->isComplete()) {
        if (ServerManager::isConnected()) {
            m_buildsPane->downloadWidget()->buttonSlice()->get(DownloadWidget::New)->setEnabled(false);
            m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->downloadWidget());
            BuildModel* model = static_cast<BuildModel*>(m_buildsPane->downloadWidget()->downloadList()->model());
            model->addBuildRequest(m_androidPlatformController->toCborMap());
            m_androidPlatformController->charge();
        } else {
            UtilityFunctions::showMessage(
                        m_buildsPane,
                        tr("Unable to connect to the server"),
                        tr("Please make sure you have a working internet connection."),
                        QMessageBox::Warning);
        }
    }
}

void BuildsController::onAndroidResetButtonClick()
{
    QMessageBox::StandardButton ret = UtilityFunctions::showMessage(
                m_buildsPane,
                tr("Do you want to reset the form?"),
                tr("This will reset all the fields to their initial values."),
                QMessageBox::Question,
                QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes)
        m_androidPlatformController->charge();
}

void BuildsController::onServerDisconnect()
{
    auto model = static_cast<BuildModel*>(m_buildsPane->downloadWidget()->downloadList()->model());
    for (int row = 0; row < model->rowCount(); ++row) {
        const QModelIndex& index = model->index(row);
        const QVariant& state = model->data(index, BuildModel::StateRole);
        if (state.isValid()) {
            if (state.toInt() != BuildModel::Finished) {
                model->setData(index, tr("Connection lost"), Qt::StatusTipRole);
                model->setData(index, true, BuildModel::ErrorRole);
                model->setData(index, BuildModel::Finished, BuildModel::StateRole);
            }
        }
    }
    m_buildsPane->downloadWidget()->buttonSlice()->get(DownloadWidget::New)->setEnabled(true);
}

QWidget* BuildsController::widgetForPlatform(Platform platform) const
{
    if (platform == Android)
        return m_buildsPane->androidPlatformWidget();
    return nullptr;
}
