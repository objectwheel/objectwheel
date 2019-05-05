#include <forgetwidget.h>
#include <buttonslice.h>
#include <bulkedit.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <utilityfunctions.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

enum Fields { Email };
enum Buttons { Next, Back };

ForgetWidget::ForgetWidget(QWidget* parent) : QWidget(parent)
  , m_layout(new QVBoxLayout(this))
  , m_iconLabel(new QLabel)
  , m_forgotLabel(new QLabel)
  , m_buttons(new ButtonSlice)
  , m_bulkEdit(new BulkEdit)
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
{
    m_layout->setSpacing(6);
    m_layout->addStretch();
    m_layout->addWidget(m_iconLabel, 0 , Qt::AlignCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_forgotLabel, 0 , Qt::AlignCenter);
    m_layout->addSpacing(50);
    m_layout->addWidget(m_bulkEdit, 0 , Qt::AlignCenter);
    m_layout->addSpacing(10);
    m_layout->addWidget(m_buttons, 0 , Qt::AlignCenter);
    m_layout->addStretch();
    m_layout->addWidget(m_loadingIndicator, 0 , Qt::AlignCenter);
    m_layout->addStretch();

    QPixmap p(":/images/forgot.png");
    p.setDevicePixelRatio(devicePixelRatioF());

    m_iconLabel->setFixedSize(QSize(48, 48));
    m_iconLabel->setPixmap(
        p.scaled(
            QSize(48, 48) * devicePixelRatioF(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    m_forgotLabel->setFont(f);
    m_forgotLabel->setText(tr("Password Reset"));

    m_bulkEdit->add(Email, tr("Email Address"));
    m_bulkEdit->setFixedWidth(350);
    m_bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_buttons->add(Back, "#5BC5F8", "#2592F9");
    m_buttons->add(Next, "#8BBB56", "#6EA045");
    m_buttons->get(Next)->setText(tr("Next"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Next)->setIcon(QIcon(":/images/load.png"));
    m_buttons->get(Back)->setIcon(QIcon(":/images/unload.png"));
    m_buttons->get(Next)->setIconSize(QSize(16, 16));
    m_buttons->get(Back)->setIconSize(QSize(16, 16));
    m_buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = 175;
    m_buttons->triggerSettings();

    connect(m_buttons->get(Back), &QPushButton::clicked, this, &ForgetWidget::back);
    connect(m_buttons->get(Next), &QPushButton::clicked, this, &ForgetWidget::onNextClicked);

    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resetPasswordSuccessful,
            this, &ForgetWidget::onResetPasswordSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resetPasswordFailure,
            this, &ForgetWidget::onResetPasswordFailure);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &ForgetWidget::unlock);

    m_loadingIndicator->setStyleSheet("background: transparent;");
    m_loadingIndicator->setColor(palette().text().color());
    m_loadingIndicator->setRoundness(50);
    m_loadingIndicator->setMinimumTrailOpacity(5);
    m_loadingIndicator->setTrailFadePercentage(100);
    m_loadingIndicator->setRevolutionsPerSecond(2);
    m_loadingIndicator->setNumberOfLines(12);
    m_loadingIndicator->setLineLength(5);
    m_loadingIndicator->setInnerRadius(4);
    m_loadingIndicator->setLineWidth(2);
}

void ForgetWidget::clear()
{
    m_bulkEdit->get<QLineEdit*>(Email)->setText("");
}

void ForgetWidget::lock()
{
    m_bulkEdit->setDisabled(true);
    m_buttons->setDisabled(true);
    m_loadingIndicator->start();
}

void ForgetWidget::unlock()
{
    m_bulkEdit->setEnabled(true);
    m_buttons->setEnabled(true);
    m_loadingIndicator->stop();
}

void ForgetWidget::onNextClicked()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();

    if (email.isEmpty() || email.size() > 255 ||
        !UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(
                    this, tr("Incorrect information"),
                    tr("Please checkout the email address you entered."));
        return;
    }

    if (!ServerManager::isConnected()) {
        UtilityFunctions::showMessage(
                    this, tr("No connection"),
                    tr("Unable to connect to the server, please checkout your internet connection."));
        return;
    }

    lock();

    RegistrationApiManager::resetPassword(m_bulkEdit->get<QLineEdit*>(Email)->text());
}

void ForgetWidget::onResetPasswordSuccessful()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    unlock();
    emit done(email);
    clear();
}

void ForgetWidget::onResetPasswordFailure()
{
    unlock();
    UtilityFunctions::showMessage(
                this, tr("Password reset failed"),
                tr("Please checkout the information you provided."));
}
