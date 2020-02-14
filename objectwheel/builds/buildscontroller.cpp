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
    connect(m_buildsPane->stackedLayout(), &StackedLayout::currentChanged,
            this, &BuildsController::onCurrentWidgetChange);
}

void BuildsController::charge()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->androidPlatformWidget());
    m_androidPlatformController->charge();
}

void BuildsController::discharge()
{
    m_buildsPane->stackedLayout()->setCurrentWidget(m_buildsPane->downloadWidget());
    m_androidPlatformController->discharge();
}

void BuildsController::onNextButtonClick()
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

void BuildsController::onCurrentWidgetChange(int index)
{
    const QWidget* currentWidget = m_buildsPane->stackedLayout()->widget(index);
    if (currentWidget == m_buildsPane->androidPlatformWidget()) {
        m_buildsPane->buttonSlice()->get(BuildsPane::Next)->setText(tr("Build"));
        m_buildsPane->platformLabel()->setText(tr("Target platform: Android"));
        m_buildsPane->descriptionLabel()->setText(tr("Please make your final adjustments down below before requesting a cloud build.\n"
                                                     "All fields are optional and will be assigned to their default values if left as is."));
        m_buildsPane->iconLabel()->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/builds/android.svg"),
                                                                QSize(60, 60), m_buildsPane->iconLabel()));
        m_buildsPane->iconLabel()->show();
        m_buildsPane->platformLabel()->show();
        m_buildsPane->descriptionLabel()->show();
    } else {
        m_buildsPane->buttonSlice()->get(BuildsPane::Next)->setText(tr("Next"));
        m_buildsPane->iconLabel()->hide();
        m_buildsPane->platformLabel()->hide();
        m_buildsPane->descriptionLabel()->hide();
    }
}
