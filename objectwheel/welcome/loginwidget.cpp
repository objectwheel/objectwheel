#include <loginwidget.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <usermanager.h>
#include <utilityfunctions.h>
#include <saveutils.h>
#include <servermanager.h>
#include <applicationcore.h>
#include <paintutils.h>
#include <applicationstyle.h>
#include <appconstants.h>

#include <QPushButton>
#include <QSettings>
#include <QBoxLayout>
#include <QLabel>

enum Fields { Email, Password };
enum Buttons { Login, Register };

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent)
  , m_bulkEdit(new BulkEdit(this))
  , m_rememberMeSwitch(new Switch(this))
  , m_buttons(new ButtonSlice(this))
  , m_helpButton(new QPushButton(this))
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
{
    auto logoLabel = new QLabel(this);
    logoLabel->setFixedSize(QSize(160, 80));
    logoLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/logo.svg"), QSize(160, 80), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto loginLabel = new QLabel(this);
    loginLabel->setFont(f);
    loginLabel->setText(tr("Log In"));

    m_bulkEdit->add(Email, tr("Email"));
    m_bulkEdit->add(Password, tr("Password"));
    m_bulkEdit->get<QLineEdit*>(Password)->setEchoMode(QLineEdit::Password);
    m_bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Password)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_buttons->add(Register, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(Login, QLatin1String("#8BBB56"), QLatin1String("#6EA045"));
    m_buttons->get(Register)->setText(tr("Sign Up"));
    m_buttons->get(Login)->setText(tr("Log In"));
    m_buttons->get(Register)->setIcon(QIcon(QStringLiteral(":/images/welcome/new.png")));
    m_buttons->get(Login)->setIcon(QIcon(QStringLiteral(":/images/welcome/load.png")));
    m_buttons->get(Register)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Login)->setCursor(Qt::PointingHandCursor);

    ApplicationStyle::setButtonStyle(m_helpButton, ApplicationStyle::Help);
    m_helpButton->setIcon(QIcon(QStringLiteral(":/images/question.svg")));
    m_helpButton->setFixedSize(22, 22);
    m_helpButton->setCursor(Qt::PointingHandCursor);

    m_loadingIndicator->setLineWidth(2);
    m_loadingIndicator->setRoundness(50);
    m_loadingIndicator->setLineLength(5);
    m_loadingIndicator->setInnerRadius(4);
    m_loadingIndicator->setNumberOfLines(12);
    m_loadingIndicator->setMinimumTrailOpacity(5);
    m_loadingIndicator->setRevolutionsPerSecond(2);
    m_loadingIndicator->setTrailFadePercentage(100);
    m_loadingIndicator->setStyleSheet(QStringLiteral("background: transparent"));
    m_loadingIndicator->setColor(palette().text().color());

    auto rememberMeWidget = new QWidget(this);
    rememberMeWidget->setFixedSize(m_bulkEdit->sizeHint().width(), 35);
    rememberMeWidget->setObjectName(QStringLiteral("rememberMeWidget"));
    rememberMeWidget->setStyleSheet(QStringLiteral("#rememberMeWidget {"
                                                   "    border-radius: 17;"
                                                   "    background: #12000000;"
                                                   "    border: 1px solid #18000000;"
                                                   "}"));

    auto rememberMeLayout = new QHBoxLayout(rememberMeWidget);
    rememberMeLayout->setSpacing(5);
    rememberMeLayout->setContentsMargins(2, 0, 0, 0);
    rememberMeLayout->addWidget(m_rememberMeSwitch, 0, Qt::AlignVCenter);
    rememberMeLayout->addWidget(new QLabel(tr("Remember me"), rememberMeWidget), 0, Qt::AlignVCenter);
    rememberMeLayout->addStretch();

    auto legalLabel = new QLabel(this);
    legalLabel->setAlignment(Qt::AlignHCenter);
    legalLabel->setText(QStringLiteral("<p><b>© 2015 - %1 %2 All Rights Reserved.</b></p>")
                        .arg(QDate::currentDate().year())
                        .arg(AppConstants::COMPANY_FULL));

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 4, 0, 4);
    layout->addStretch();
    layout->addStretch();
    layout->addWidget(logoLabel, 0, Qt::AlignHCenter);
    layout->addWidget(loginLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(rememberMeWidget, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addWidget(m_helpButton, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_loadingIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(legalLabel, 0, Qt::AlignHCenter);

    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &LoginWidget::onServerDisconnected);
    connect(UserManager::instance(), &UserManager::loggedIn,
            this, &LoginWidget::onLoginSuccessful);
    connect(UserManager::instance(), &UserManager::loginFailed,
            this, &LoginWidget::onLoginFailure);
    connect(m_buttons->get(Register), &QPushButton::clicked,
            this, &LoginWidget::signup);
    connect(m_buttons->get(Login), &QPushButton::clicked,
            this, &LoginWidget::onLoginButtonClicked);
    connect(m_helpButton, &QPushButton::clicked,
            this, &LoginWidget::onHelpButtonClicked);

    restoreRememberMe();
}

void LoginWidget::onHelpButtonClicked()
{
    const QMessageBox::StandardButton ret = UtilityFunctions::showMessage(
                this, tr("Need help?"), tr("Do you want to reset your password?"),
                QMessageBox::Question, QMessageBox::Reset | QMessageBox::Help |
                QMessageBox::Close, QMessageBox::Close);
    if (ret == QMessageBox::Help)
        emit about();
    else if (ret == QMessageBox::Reset)
        emit resetPassword();
}

void LoginWidget::onLoginButtonClicked()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    const QString& password = m_bulkEdit->get<QLineEdit*>(Password)->text();
    const QString& hash = UtilityFunctions::isPasswordHashFormatCorrect(password)
            ? password : UtilityFunctions::hashPassword(password);

    if (email.isEmpty() || password.isEmpty()) {
        UtilityFunctions::showMessage(this,
                                      tr("Fields cannot be left blank"),
                                      tr("Please fill in all the required fields."),
                                      QMessageBox::Information);
        return;
    }

    if (email.size() > 255 || password.size() > 255) {
        UtilityFunctions::showMessage(this,
                                      tr("Entry too long"),
                                      tr("No fields can be larger than 255 characters."),
                                      QMessageBox::Information);
        return;
    }

    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt email address"),
                                      tr("Your email address doesn't comply with "
                                         "the standard email address format."),
                                      QMessageBox::Information);
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password) && password != hash) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt password"),
                                      tr("Your password must comply with following standards:\n"
                                         "•  Length must be between 6 and 35 characters\n"
                                         "•  Only Latin-1 characters are allowed\n"
                                         "•  Whitespace characters are not allowed\n"
                                         "•  It can contain a-z, A-Z, 0-9\n"
                                         "•  It can also contain following special characters:\n"
                                         "   [ ] > < { } * ! @ - # $ % ^ & + = ~ . , :"),
                                      QMessageBox::Information);
        return;
    }

    if (!ServerManager::isConnected()) {
        if (UserManager::hasLocalData(email)) {
            const QDateTime& lastOnline = SaveUtils::userLastOnlineDate(UserManager::dir(email));
            PlanManager::Plans plan = static_cast<PlanManager::Plans>(SaveUtils::userPlan(UserManager::dir(email)));
            if (lastOnline.daysTo(QDateTime::currentDateTime()) > 30) {
                UtilityFunctions::showMessage(this,
                                              tr("You have reached the offline usage limit"),
                                              tr("Please connect to the Internet in order to "
                                                 "continue using %1 in offline mode.").arg(AppConstants::NAME));
            } else if (!PlanManager::isEligibleForOfflineLogging(plan)) {
                UtilityFunctions::showMessage(this,
                                              tr("You are not eligible for offline mode"),
                                              tr("Please upgrade your plan in order to enable offline "
                                                 "mode or checkout your internet connection."));
            } else {
                m_loadingIndicator->start();
                UserManager::loginOffline(email, hash);
            }
        } else {
            UtilityFunctions::showMessage(this,
                                          tr("Unable to connect to the server"),
                                          tr("We could not find any local data to enable offline "
                                             "login for the issued email address, please connect "
                                             "to the internet."),
                                          QMessageBox::Information);
        }
    } else {
        m_loadingIndicator->start();
        UserManager::login(email, hash);
    }
}

void LoginWidget::onLoginSuccessful()
{
    if (m_rememberMeSwitch->isChecked())
        saveRememberMe();
    else
        clearRememberMe();
    m_loadingIndicator->stop();
    m_bulkEdit->get<QLineEdit*>(Email)->clear();
    m_bulkEdit->get<QLineEdit*>(Password)->clear();
    emit done();
}

void LoginWidget::onLoginFailure()
{
    clearRememberMe();
    m_loadingIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Unable to log in"),
                                  tr("Incorrect email address or password, "
                                     "please checkout the information you entered."));
}

void LoginWidget::onServerDisconnected()
{
    if (m_loadingIndicator->isSpinning()) {
        m_loadingIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}

void LoginWidget::saveRememberMe()
{
    QSettings* settings = ApplicationCore::settings();
    Q_ASSERT(settings);
    Q_ASSERT(UserManager::isLoggedIn());
    settings->setValue(QStringLiteral("User/Security.RememberMe"),
                       UtilityFunctions::generateAutoLoginHash(UserManager::email(),
                                                               UserManager::password()));
}

void LoginWidget::clearRememberMe()
{
    QSettings* settings = ApplicationCore::settings();
    Q_ASSERT(settings);
    settings->setValue(QStringLiteral("User/Security.RememberMe"),
                       UtilityFunctions::generateJunk(999));
    settings->sync();
    settings->setValue(QStringLiteral("User/Security.RememberMe"), QVariant());
}

void LoginWidget::restoreRememberMe()
{
    QString em, pw;
    if (UtilityFunctions::testAutoLogin(
                ApplicationCore::settings()->
                value(QStringLiteral("User/Security.RememberMe")).toByteArray(), &em, &pw)) {
        m_bulkEdit->get<QLineEdit*>(Email)->setText(em);
        m_bulkEdit->get<QLineEdit*>(Password)->setText(pw);
        m_rememberMeSwitch->setChecked(true);
    } else {
        m_rememberMeSwitch->setChecked(false);
    }
}
