#include <resetpasswordwidget.h>
#include <buttonslice.h>
#include <bulkedit.h>
#include <busyindicatorwidget.h>
#include <apimanager.h>
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
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
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
    m_buttons->settings().cellWidth = m_bulkEdit->sizeHint().width() / 2.0;
    m_buttons->triggerSettings();

    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setMinimumTrailOpacity(5);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setNumberOfLines(12);
    m_busyIndicator->setLineLength(5);
    m_busyIndicator->setInnerRadius(4);
    m_busyIndicator->setLineWidth(2);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addStretch();
    layout->addWidget(iconLabel, 0 , Qt::AlignHCenter);
    layout->addWidget(forgotLabel, 0 , Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0 , Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0 , Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_busyIndicator, 0 , Qt::AlignHCenter);
    layout->addStretch();

    connect(m_buttons->get(Back), &QPushButton::clicked,
            this, &ResetPasswordWidget::back);
    connect(m_buttons->get(Next), &QPushButton::clicked,
            this, &ResetPasswordWidget::onNextClicked);
    connect(ApiManager::instance(), &ApiManager::resetPasswordSuccessful,
            this, &ResetPasswordWidget::onResetPasswordSuccessful);
    connect(ApiManager::instance(), &ApiManager::resetPasswordFailure,
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
        m_busyIndicator->start();
        ApiManager::resetPassword(email);
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
    m_busyIndicator->stop();
    m_bulkEdit->get<QLineEdit*>(Email)->setText(QString());
    emit done(email);
}

void ResetPasswordWidget::onResetPasswordFailure()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Invalid information entered"),
                                  tr("The server rejected your request. Please review the information you entered "
                                     "and make sure you are not trying to reset passwords too much."));
}

void ResetPasswordWidget::onServerDisconnected()
{
    if (m_busyIndicator->isSpinning()) {
        m_busyIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}
