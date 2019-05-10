#include <loginwidget.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <flatbutton.h>
#include <waitingspinnerwidget.h>
#include <usermanager.h>
#include <utilityfunctions.h>
#include <saveutils.h>
#include <servermanager.h>
#include <applicationcore.h>

#include <QGridLayout>
#include <QLabel>
#include <QSettings>

enum Fields { Email, Password };
enum Buttons { Login, Register };

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent)
  , m_layout(new QGridLayout(this))
  , m_logoLabel(new QLabel(this))
  , m_loginLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
  , m_rememberMeSwitch(new Switch(this))
  , m_rememberMeLabel(new QLabel(this))
  , m_buttons(new ButtonSlice(this))
  , m_helpButton(new FlatButton(this))
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
  , m_legalLabel(new QLabel(this))
{
    auto rememberMeWidget = new QWidget(this);
    auto rememberMeLayout = new QHBoxLayout(rememberMeWidget);

    m_layout->setSpacing(6);
    m_layout->setRowStretch(0, 1);
    m_layout->setRowStretch(1, 1);
    m_layout->setRowStretch(8, 1);
    m_layout->setRowStretch(10, 1);
    m_layout->setColumnStretch(0, 1);
    m_layout->setColumnStretch(2, 1);

    m_layout->addWidget(m_logoLabel, 2, 1, Qt::AlignCenter);
    m_layout->addWidget(m_loginLabel, 3, 1, Qt::AlignCenter);
    m_layout->addWidget(m_bulkEdit, 4, 1, Qt::AlignCenter);
    m_layout->addWidget(rememberMeWidget, 5, 1, Qt::AlignCenter);
    m_layout->addWidget(m_buttons, 6, 1, Qt::AlignCenter);
    m_layout->addWidget(m_helpButton, 7, 1, Qt::AlignCenter);
    m_layout->addWidget(m_loadingIndicator, 9, 1, Qt::AlignCenter);
    m_layout->addWidget(m_legalLabel, 11, 1, Qt::AlignCenter);

    rememberMeLayout->setSpacing(5);
    rememberMeLayout->setContentsMargins(2, 0, 0, 0);
    rememberMeLayout->addWidget(m_rememberMeSwitch);
    rememberMeLayout->addWidget(m_rememberMeLabel);
    rememberMeLayout->setAlignment(m_rememberMeLabel, Qt::AlignVCenter);
    rememberMeLayout->setAlignment(m_rememberMeSwitch, Qt::AlignVCenter);
    rememberMeLayout->addStretch();
    rememberMeWidget->setFixedSize(300, 35);
    rememberMeWidget->setObjectName("rememberMeWidget");
    rememberMeWidget->setStyleSheet(QStringLiteral("#rememberMeWidget {"
                                                   "    border-radius: 17;"
                                                   "    background: #12000000;"
                                                   "    border: 1px solid #18000000;"
                                                   "}"));

    m_rememberMeLabel->setText(tr("Remember me"));

    m_legalLabel->setAlignment(Qt::AlignHCenter);
    m_legalLabel->setText(QString("<p><b>© 2015 - %1 %2 All Rights Reserved.</b></p>")
                          .arg(QDate::currentDate().year()).arg(APP_CORP));

    m_helpButton->settings().iconButton = true;
    m_helpButton->setIcon(QIcon(":/images/question.png"));
    m_helpButton->setFixedSize(20,20);

    QPixmap p(":/images/logo.png");
    p.setDevicePixelRatio(devicePixelRatioF());
    m_logoLabel->setFixedSize(QSize(160, 80));
    m_logoLabel->setPixmap(p.scaled(QSize(160, 80) * devicePixelRatioF(), Qt::IgnoreAspectRatio,
                                    Qt::SmoothTransformation));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);
    m_loginLabel->setFont(f);
    m_loginLabel->setText(tr("Log In"));

    m_bulkEdit->add(Email, tr("Email"));
    m_bulkEdit->add(Password, tr("Password"));
    m_bulkEdit->get<QLineEdit*>(Password)->setEchoMode(QLineEdit::Password);
    m_bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Password)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_buttons->add(Register, "#5BC5F8", "#2592F9");
    m_buttons->add(Login, "#8BBB56", "#6EA045");
    m_buttons->get(Register)->setText(tr("Sign Up"));
    m_buttons->get(Login)->setText(tr("Log In"));
    m_buttons->get(Register)->setIcon(QIcon(":/images/new.png"));
    m_buttons->get(Login)->setIcon(QIcon(":/images/load.png"));
    m_buttons->get(Register)->setIconSize(QSize(16, 16));
    m_buttons->get(Login)->setIconSize(QSize(16, 16));
    m_buttons->get(Register)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Login)->setCursor(Qt::PointingHandCursor);

    m_loadingIndicator->setLineWidth(2);
    m_loadingIndicator->setRoundness(50);
    m_loadingIndicator->setLineLength(5);
    m_loadingIndicator->setInnerRadius(4);
    m_loadingIndicator->setNumberOfLines(12);
    m_loadingIndicator->setMinimumTrailOpacity(5);
    m_loadingIndicator->setRevolutionsPerSecond(2);
    m_loadingIndicator->setTrailFadePercentage(100);
    m_loadingIndicator->setStyleSheet("background: transparent;");
    m_loadingIndicator->setColor(palette().text().color());

    connect(UserManager::instance(), &UserManager::loggedIn,
            this, &LoginWidget::onLoginSuccessful);
    connect(UserManager::instance(), &UserManager::loginFailed,
            this, &LoginWidget::onLoginFailure);
    connect(m_buttons->get(Register), &QPushButton::clicked,
            this, &LoginWidget::signup);
    connect(m_buttons->get(Login), &QPushButton::clicked,
            this, &LoginWidget::onLoginButtonClick);
    connect(m_helpButton, &FlatButton::clicked, this, [=] {
        auto ret = UtilityFunctions::showMessage(
                    this, tr("Need help?"), tr("Do you want to reset your password?"),
                    QMessageBox::Question, QMessageBox::Reset | QMessageBox::Help |
                    QMessageBox::Close, QMessageBox::Close);
        if (ret == QMessageBox::Help)
            return emit about();
        if (ret == QMessageBox::Reset)
            return emit forget();
    });

    restoreRememberMe();
}

void LoginWidget::clear()
{
    m_bulkEdit->get<QLineEdit*>(Email)->clear();
    m_bulkEdit->get<QLineEdit*>(Password)->clear();
}

void LoginWidget::lock()
{
    m_bulkEdit->setDisabled(true);
    m_buttons->setDisabled(true);
    m_rememberMeSwitch->setDisabled(true);
    m_helpButton->setDisabled(true);
    m_loadingIndicator->start();
}

void LoginWidget::unlock()
{
    m_bulkEdit->setEnabled(true);
    m_buttons->setEnabled(true);
    m_rememberMeSwitch->setEnabled(true);
    m_helpButton->setEnabled(true);
    m_loadingIndicator->stop();
}

void LoginWidget::clearRememberMe()
{
    QSettings* settings = ApplicationCore::settings();
    Q_ASSERT(settings);
    settings->setValue(QStringLiteral("User/Security.RememberMe"),
                       UtilityFunctions::generateJunk(9999));
    settings->sync();
    settings->setValue(QStringLiteral("User/Security.RememberMe"), QVariant());
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

void LoginWidget::restoreRememberMe()
{
    QString em, pw;
    if (UtilityFunctions::testAutoLogin(
                ApplicationCore::settings()->
                value(QStringLiteral("User/Security.RememberMe")).toByteArray(), &em, &pw)) {
        m_bulkEdit->get<QLineEdit*>(Email)->setText(em);
        m_bulkEdit->get<QLineEdit*>(Password)->setText(pw);
        m_rememberMeSwitch->setChecked(true);
    }
}

void LoginWidget::onLoginButtonClick()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    const QString& password = m_bulkEdit->get<QLineEdit*>(Password)->text();
    const QString& hash = UtilityFunctions::isPasswordHashFormatCorrect(password)
            ? password : UserManager::hashPassword(password);

    if (email.isEmpty() || password.isEmpty()) {
        UtilityFunctions::showMessage(this,
                                      tr("Email and password fields cannot be left blank"),
                                      tr("Please fill in the required fields first."));
        return;
    }

    if (email.size() > 255 || password.size() > 255) {
        UtilityFunctions::showMessage(this,
                                      tr("Inputs are too long"),
                                      tr("The length of any fields cannot exceed 255 characters."));
        return;
    }

    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt email address"),
                                      tr("Your email address doesn't comply with "
                                         "the standard email address format."));
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password)
            && !UtilityFunctions::isPasswordHashFormatCorrect(password)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt password"),
                                      tr("Your password must comply with following standards:\n"
                                         "•  Length must be between 6 and 35 characters\n"
                                         "•  Only Latin-1 characters are allowed\n"
                                         "•  Whitespace characters are not allowed\n"
                                         "•  It can contain a-z, A-Z, 0-9\n"
                                         "•  It can also contain following special characters:\n"
                                         "   [ ] > < { } * ! @ - # $ % ^ & + = ~ . , :"));
        return;
    }

    if (!ServerManager::isConnected()) {
        if (UserManager::hasLocalData(email)) {
            PlanManager::Plans plan = static_cast<PlanManager::Plans>(SaveUtils::userPlan(UserManager::dir(email)));
            if (!PlanManager::isEligibleForOfflineLogging(plan)) {
                UtilityFunctions::showMessage(this,
                                              tr("No connection"),
                                              tr("Unable to connect to the server, please checkout "
                                                 "your internet connection or upgrade your account "
                                                 "to a higher plan in order to enable offline mode."));
                return;
            }

            lock();

            UserManager::loginOffline(email, hash);

            return;
        } else {
            UtilityFunctions::showMessage(this,
                                          tr("No connection"),
                                          tr("Unable to connect to the server, please checkout "
                                             "your internet connection. Also we couldn't find "
                                             "any local data to enable offline mode. You must "
                                             "login to your account via using internet for the "
                                             "first time in order to enable offline mode."));
            return;
        }
    }

    lock();

    UserManager::login(email, hash);
}

void LoginWidget::onLoginSuccessful()
{
    if (m_rememberMeSwitch->isChecked())
        saveRememberMe();
    else
        clearRememberMe();

    unlock();

    QTimer::singleShot(100, this, &LoginWidget::clear);

    emit done();
}

void LoginWidget::onLoginFailure()
{
    clearRememberMe();

    unlock();

    UtilityFunctions::showMessage(this,
                                  tr("Unable to login"),
                                  tr("Incorrect information, please "
                                     "checkout the information you entered."));
}
