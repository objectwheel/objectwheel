#include <buildscontroller.h>
#include <buildspane.h>
#include <buttonslice.h>
#include <stackedlayout.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <androidplatformwidget.h>
#include <androidplatformcontroller.h>
#include <downloadwidget.h>
#include <downloadcontroller.h>

#include <QPushButton>
#include <QLabel>

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
    connect(m_buildsPane->androidPlatformWidget()->buttonSlice()->get(AndroidPlatformWidget::Back), &QPushButton::clicked,
            this, &BuildsController::onBackButtonClick);
    connect(m_buildsPane->androidPlatformWidget()->buttonSlice()->get(AndroidPlatformWidget::Build), &QPushButton::clicked,
            this, &BuildsController::onBuildButtonClick);
}

void BuildsController::charge()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->downloadWidget());
    m_androidPlatformController->charge();
}

void BuildsController::discharge()
{
    m_androidPlatformController->discharge();
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

void BuildsController::onBuildButtonClick()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->downloadWidget());
}

QWidget* BuildsController::widgetForPlatform(PlatformSelectionWidget::Platform platform) const
{
    if (platform == PlatformSelectionWidget::Android)
        return m_buildsPane->androidPlatformWidget();
    return nullptr;
}
