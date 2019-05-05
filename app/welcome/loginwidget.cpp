#include <loginwidget.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <flatbutton.h>
#include <waitingspinnerwidget.h>
#include <global.h>
#include <usermanager.h>
#include <async.h>
#include <utilityfunctions.h>
#include <servermanager.h>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>

#define AUTOLOGIN_HEIGHT (35)
#define AUTOLOGIN_WIDTH  (300)
#define SIZE_LOGO        (QSize(160, 80))
#define PATH_LOGO        (":/images/logo.png")
#define PATH_RICON       (":/images/new.png")
#define PATH_LICON       (":/images/load.png")
#define PATH_HICON       (":/images/question.png")

enum Fields { Email, Password };
enum Buttons { Login, Register };

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent)
{
    m_layout = new QGridLayout(this);
    m_logoLabel = new QLabel;
    m_loginLabel = new QLabel;
    m_bulkEdit = new BulkEdit;
    m_autologinWidget = new QWidget;
    m_autologinLayout = new QHBoxLayout(m_autologinWidget);
    m_autologinSwitch = new Switch;
    m_autologinLabel = new QLabel;
    m_buttons = new ButtonSlice;
    m_helpButton = new FlatButton;
    m_loadingIndicator = new WaitingSpinnerWidget(this, false);
    m_legalLabel = new QLabel;

    m_layout->setSpacing(6);
    m_layout->setRowStretch(0, 1);
    m_layout->setRowStretch(1, 1);
    m_layout->setRowStretch(8, 1);
    m_layout->setRowStretch(10, 1);
    m_layout->setColumnStretch(0, 1);
    m_layout->setColumnStretch(2, 1);

    m_layout->addWidget(m_logoLabel, 2, 1);
    m_layout->addWidget(m_loginLabel, 3, 1);
    m_layout->addWidget(m_bulkEdit, 4, 1);
    m_layout->addWidget(m_autologinWidget, 5, 1);
    m_layout->addWidget(m_buttons, 6, 1);
    m_layout->addWidget(m_helpButton, 7, 1);
    m_layout->addWidget(m_loadingIndicator, 9, 1);
    m_layout->addWidget(m_legalLabel, 11, 1);
    m_layout->setAlignment(m_logoLabel, Qt::AlignCenter);
    m_layout->setAlignment(m_loginLabel, Qt::AlignCenter);
    m_layout->setAlignment(m_bulkEdit, Qt::AlignCenter);
    m_layout->setAlignment(m_autologinWidget, Qt::AlignCenter);
    m_layout->setAlignment(m_buttons, Qt::AlignCenter);
    m_layout->setAlignment(m_helpButton, Qt::AlignCenter);
    m_layout->setAlignment(m_loadingIndicator, Qt::AlignCenter);
    m_layout->setAlignment(m_legalLabel, Qt::AlignCenter);

    m_autologinLayout->setSpacing(5);
    m_autologinLayout->setContentsMargins(2, 0, 0, 0);
    m_autologinLayout->addWidget(m_autologinSwitch);
    m_autologinLayout->addWidget(m_autologinLabel);
    m_autologinLayout->setAlignment(m_autologinLabel, Qt::AlignVCenter);
    m_autologinLayout->setAlignment(m_autologinSwitch, Qt::AlignVCenter);
    m_autologinLayout->addStretch();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(devicePixelRatioF());

    m_logoLabel->setFixedSize(SIZE_LOGO);
    m_logoLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * devicePixelRatioF(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

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

    m_autologinWidget->setObjectName("autologinWidget");
    m_autologinWidget->setFixedSize(AUTOLOGIN_WIDTH, AUTOLOGIN_HEIGHT);
    m_autologinWidget->setStyleSheet(
        tr(
            "#autologinWidget {"
            "    border-radius: %1;"
            "    background: #12000000;"
            "    border: 1px solid #18000000;"
            "}"
        )
        .arg(int(AUTOLOGIN_HEIGHT / 2.0))
    );

    m_autologinLabel->setText(tr("Automatic login"));
    m_autologinSwitch->setChecked(true);

    m_buttons->add(Register, "#5BC5F8", "#2592F9");
    m_buttons->add(Login, "#8BBB56", "#6EA045");
    m_buttons->get(Register)->setText(tr("Sign Up"));
    m_buttons->get(Login)->setText(tr("Log In"));
    m_buttons->get(Register)->setIcon(QIcon(PATH_RICON));
    m_buttons->get(Login)->setIcon(QIcon(PATH_LICON));
    m_buttons->get(Register)->setIconSize(QSize(16, 16));
    m_buttons->get(Login)->setIconSize(QSize(16, 16));
    m_buttons->get(Register)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Login)->setCursor(Qt::PointingHandCursor);
    connect(m_buttons->get(Register), &QPushButton::clicked,
      this, &LoginWidget::signup);
    connect(m_buttons->get(Login), &QPushButton::clicked,
      this, &LoginWidget::onLoginButtonClick);
    connect(&m_encryptionWatcher, &QFutureWatcher<bool>::finished,
      this, &LoginWidget::onSessionStart);

    m_helpBox = new QMessageBox(this);
    m_helpBox->setIcon(QMessageBox::Question);
    m_helpBox->setWindowTitle("Help");
    m_helpBox->setText("Need help?");
    m_helpBox->addButton("Forgot my password", QMessageBox::ActionRole);
    m_helpBox->addButton("About", QMessageBox::ActionRole);
    m_helpBox->addButton(QMessageBox::Cancel);
    m_helpBox->setDefaultButton(QMessageBox::Cancel);
    connect(m_helpBox, (void(QMessageBox::*)(QAbstractButton*))
      (&QMessageBox::buttonClicked), this, [=] (QAbstractButton* button) {
        if (button->text().contains("Forgot"))
            emit forget();
        else if (button->text().contains("About"))
            emit about();
    });

    m_helpButton->settings().iconButton = true;
    m_helpButton->setIcon(QIcon(PATH_HICON));
    m_helpButton->setFixedSize(20,20);
    connect(m_helpButton, &FlatButton::clicked, m_helpBox, &QMessageBox::show);

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

    m_legalLabel->setText(QString("<p><b>© 2015 - 2019 %1 All Rights Reserved.</b></p>").arg(APP_CORP));
    m_legalLabel->setAlignment(Qt::AlignHCenter);

    connect(RegistrationApiManager::instance(), &RegistrationApiManager::loginSuccessful,
            this, &LoginWidget::onLoginSuccessful);
    connect(RegistrationApiManager::instance(), &RegistrationApiManager::loginFailure,
            this, &LoginWidget::onLoginFailure);
    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &LoginWidget::unlock);
}

void LoginWidget::lock()
{
    m_bulkEdit->setDisabled(true);
    m_buttons->setDisabled(true);
    m_autologinSwitch->setDisabled(true);
    m_helpButton->setDisabled(true);
    m_loadingIndicator->start();
}

void LoginWidget::unlock()
{
    m_bulkEdit->setEnabled(true);
    m_buttons->setEnabled(true);
    m_autologinSwitch->setEnabled(true);
    m_helpButton->setEnabled(true);
    m_loadingIndicator->stop();
}

void LoginWidget::clear()
{
    m_bulkEdit->get<QLineEdit*>(Email)->clear();
    m_bulkEdit->get<QLineEdit*>(Password)->clear();
}

void LoginWidget::onLoginButtonClick()
{
    auto email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    auto password = m_bulkEdit->get<QLineEdit*>(Password)->text();

    if (email.isEmpty() || email.size() > 255 ||
        password.isEmpty() || password.size() > 255 ||
        !UtilityFunctions::isEmailFormatCorrect(email) ||
        !UtilityFunctions::isPasswordFormatCorrect(password)) {
        UtilityFunctions::showMessage(this, tr("Incorrect information"),
                                      tr("The information you provided is incorrect."));
        return;
    }

    if (!ServerManager::isConnected()) {
        UtilityFunctions::showMessage(
                    this, tr("No connection"),
                    tr("Unable to connect to the server, please checkout your internet connection."));
        return;
    }

    lock();

    RegistrationApiManager::login(email, password);
}

void LoginWidget::onLoginSuccessful(const RegistrationApiManager::Plans& /*plan*/)
{
    unlock();
    QTimer::singleShot(0, this, &LoginWidget::startSession);
    emit busy(tr("Decryption in progress"));
}

void LoginWidget::onLoginFailure()
{
    unlock();
    UtilityFunctions::showMessage(
                this, tr("Unable to login"),
                tr("Incorrect information, please checkout the information you entered."));
}

void LoginWidget::startSession()
{
    const QString email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    const QString password = m_bulkEdit->get<QLineEdit*>(Password)->text();

    UserManager::newUser(email);
    QFuture<bool> future = Async::run(QThreadPool::globalInstance(), &UserManager::start, email, password);

    m_encryptionWatcher.setFuture(future);
}

void LoginWidget::onSessionStart()
{
    auto password = m_bulkEdit->get<QLineEdit*>(Password)->text();

    if (m_encryptionWatcher.result()) {
        if (m_autologinSwitch->isChecked())
            UserManager::setAutoLogin(password);
        else
            UserManager::clearAutoLogin();
    } else
        qFatal("Fatal : LoginWidget");

    QTimer::singleShot(500, this, &LoginWidget::clear);

    emit done();
}
