#include <resetpasswordwidget.h>
#include <buttonslice.h>
#include <bulkedit.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <servermanager.h>

#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

enum Fields { Email };
enum Buttons { Next, Back };

ResetPasswordWidget::ResetPasswordWidget(QWidget* parent) : QWidget(parent)
  , m_bulkEdit(new BulkEdit(this))
  , m_buttons(new ButtonSlice(this))
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/forget.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto forgotLabel = new QLabel(this);
    forgotLabel->setFont(f);
    forgotLabel->setText(tr("Password Reset"));

    m_bulkEdit->setFixedWidth(350);
    m_bulkEdit->add(Email, tr("Email Address"));
    m_bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(Next, QLatin1String("#8BBB56"), QLatin1String("#6EA045"));
    m_buttons->get(Next)->setText(tr("Next"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Next)->setIcon(QIcon(QStringLiteral(":/images/welcome/load.png")));
    m_buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    m_buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = m_bulkEdit->width() / 2.0;
    m_buttons->triggerSettings();

    m_loadingIndicator->setStyleSheet(QStringLiteral("background: transparent"));
    m_loadingIndicator->setColor(palette().text().color());
    m_loadingIndicator->setRoundness(50);
    m_loadingIndicator->setMinimumTrailOpacity(5);
    m_loadingIndicator->setTrailFadePercentage(100);
    m_loadingIndicator->setRevolutionsPerSecond(2);
    m_loadingIndicator->setNumberOfLines(12);
    m_loadingIndicator->setLineLength(5);
    m_loadingIndicator->setInnerRadius(4);
    m_loadingIndicator->setLineWidth(2);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0 , Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(forgotLabel, 0 , Qt::AlignHCenter);
    layout->addSpacing(50);
    layout->addWidget(m_bulkEdit, 0 , Qt::AlignHCenter);
    layout->addSpacing(10);
    layout->addWidget(m_buttons, 0 , Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_loadingIndicator, 0 , Qt::AlignHCenter);
    layout->addStretch();

    connect(m_buttons->get(Back), &QPushButton::clicked,
            this, &ResetPasswordWidget::back);
    connect(m_buttons->get(Next), &QPushButton::clicked,
            this, &ResetPasswordWidget::onNextClicked);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resetPasswordSuccessful,
            this, &ResetPasswordWidget::onResetPasswordSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::resetPasswordFailure,
            this, &ResetPasswordWidget::onResetPasswordFailure);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &ResetPasswordWidget::onServerDisconnected);
}

void ResetPasswordWidget::onNextClicked()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();

    if (email.isEmpty()
            || email.size() > 255
            || !UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(this,
                                      tr("Invalid information entered"),
                                      tr("Please review the email address you entered."));
        return;
    }

    if (ServerManager::isConnected()) {
        m_loadingIndicator->start();
        RegistrationApiManager::resetPassword(email);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void ResetPasswordWidget::onResetPasswordSuccessful()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    m_loadingIndicator->stop();
    m_bulkEdit->get<QLineEdit*>(Email)->setText(QString());
    emit done(email);
}

void ResetPasswordWidget::onResetPasswordFailure()
{
    m_loadingIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Invalid information entered"),
                                  tr("The server rejected your request. Please review the information you entered "
                                     "and make sure you are not trying to reset passwords too much."));
}

void ResetPasswordWidget::onServerDisconnected()
{
    if (m_loadingIndicator->isSpinning()) {
        m_loadingIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}
