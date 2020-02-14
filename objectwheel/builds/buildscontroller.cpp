#include <buildscontroller.h>
#include <buildspane.h>
#include <androidplatformcontroller.h>
#include <buttonslice.h>
#include <stackedlayout.h>
#include <platformselectionwidget.h>
#include <androidplatformwidget.h>
#include <utilityfunctions.h>
#include <downloadwidget.h>
#include <paintutils.h>

#include <QPushButton>
#include <QLabel>

BuildsController::BuildsController(BuildsPane* buildsPane, QObject* parent) : QObject(parent)
  , m_buildsPane(buildsPane)
  , m_androidPlatformController(new AndroidPlatformController(m_buildsPane->androidPlatformWidget(), this))
{
    connect(m_buildsPane->buttonSlice()->get(BuildsPane::Next), &QPushButton::clicked,
            this, &BuildsController::onNextButtonClick);
    connect(m_buildsPane->buttonSlice()->get(BuildsPane::Back), &QPushButton::clicked,
            this, &BuildsController::onBackButtonClick);
}

void BuildsController::charge()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->platformSelectionWidget());
    m_androidPlatformController->charge();
}

void BuildsController::discharge()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->downloadWidget());
    m_androidPlatformController->discharge();
}

void BuildsController::onNextButtonClick()
{
    m_buildsPane->iconLabel()->hide();
    m_buildsPane->platformLabel()->hide();
    m_buildsPane->descriptionLabel()->hide();

    m_buildsPane->iconLabel()->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/gift.svg"),
                                                            QSize(60, 60), m_buildsPane->iconLabel()));

//    if (m_buildsPane->stackedLayout()->currentWidget() == m_buildsPane->platformSelectionWidget())
//        return;
//    if (m_buildsPane->stackedLayout()->currentWidget() == m_buildsPane->platformSelectionWidget()) {
//        if (m_buildsPane->platformSelectionWidget()->currentPlatform() == PlatformSelectionWidget::Android) {
//            m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->androidPlatformWidget());
//        } else {
//            UtilityFunctions::showMessage(m_buildsPane, tr("Oops!"),
//                                          tr("Select a valid platform to continue"),
//                                          QMessageBox::Warning);
//        }

//    }
}

void BuildsController::onBackButtonClick()
{
//    if (m_buildsPane->stackedLayout()->currentWidget() == m_buildsPane->platformSelectionWidget())
//        return;
//    if (m_buildsPane->stackedLayout()->currentWidget() == m_buildsPane->platformSelectionWidget()) {
//        if (m_buildsPane->platformSelectionWidget()->currentPlatform() == PlatformSelectionWidget::Android) {
//            m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->androidPlatformWidget());
//        } else {
//            UtilityFunctions::showMessage(m_buildsPane, tr("Oops!"),
//                                          tr("Select a valid platform to continue"),
//                                          QMessageBox::Warning);
//        }

//    }
}
