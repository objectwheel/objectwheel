#include <loginwidget.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <flatbutton.h>
#include <waitingspinnerwidget.h>
#include <global.h>
#include <accountmanager.h>
#include <usermanager.h>
#include <async.h>
#include <utilityfunctions.h>

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
    _layout = new QGridLayout(this);
    _logoLabel = new QLabel;
    _loginLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _autologinWidget = new QWidget;
    _autologinLayout = new QHBoxLayout(_autologinWidget);
    _autologinSwitch = new Switch;
    _autologinLabel = new QLabel;
    _buttons = new ButtonSlice;
    _helpButton = new FlatButton;
    _loadingIndicator = new WaitingSpinnerWidget(this, false);
    _legalLabel = new QLabel;

    _layout->setSpacing(6);
    _layout->setRowStretch(0, 1);
    _layout->setRowStretch(1, 1);
    _layout->setRowStretch(8, 1);
    _layout->setRowStretch(10, 1);
    _layout->setColumnStretch(0, 1);
    _layout->setColumnStretch(2, 1);

    _layout->addWidget(_logoLabel, 2, 1);
    _layout->addWidget(_loginLabel, 3, 1);
    _layout->addWidget(_bulkEdit, 4, 1);
    _layout->addWidget(_autologinWidget, 5, 1);
    _layout->addWidget(_buttons, 6, 1);
    _layout->addWidget(_helpButton, 7, 1);
    _layout->addWidget(_loadingIndicator, 9, 1);
    _layout->addWidget(_legalLabel, 11, 1);
    _layout->setAlignment(_logoLabel, Qt::AlignCenter);
    _layout->setAlignment(_loginLabel, Qt::AlignCenter);
    _layout->setAlignment(_bulkEdit, Qt::AlignCenter);
    _layout->setAlignment(_autologinWidget, Qt::AlignCenter);
    _layout->setAlignment(_buttons, Qt::AlignCenter);
    _layout->setAlignment(_helpButton, Qt::AlignCenter);
    _layout->setAlignment(_loadingIndicator, Qt::AlignCenter);
    _layout->setAlignment(_legalLabel, Qt::AlignCenter);

    _autologinLayout->setSpacing(5);
    _autologinLayout->setContentsMargins(2, 0, 0, 0);
    _autologinLayout->addWidget(_autologinSwitch);
    _autologinLayout->addWidget(_autologinLabel);
    _autologinLayout->setAlignment(_autologinLabel, Qt::AlignVCenter);
    _autologinLayout->setAlignment(_autologinSwitch, Qt::AlignVCenter);
    _autologinLayout->addStretch();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(devicePixelRatioF());

    _logoLabel->setFixedSize(SIZE_LOGO);
    _logoLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * devicePixelRatioF(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    _loginLabel->setFont(f);
    _loginLabel->setText(tr("Log In"));

    _bulkEdit->add(Email, tr("Email"));
    _bulkEdit->add(Password, tr("Password"));
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setEchoMode(QLineEdit::Password);
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    _autologinWidget->setObjectName("autologinWidget");
    _autologinWidget->setFixedSize(AUTOLOGIN_WIDTH, AUTOLOGIN_HEIGHT);
    _autologinWidget->setStyleSheet(
        tr(
            "#autologinWidget {"
            "    border-radius: %1;"
            "    background: #12000000;"
            "    border: 1px solid #18000000;"
            "}"
        )
        .arg(int(AUTOLOGIN_HEIGHT / 2.0))
    );

    _autologinLabel->setText(tr("Automatic login"));
    _autologinSwitch->setChecked(true);

    _buttons->add(Register, "#5BC5F8", "#2592F9");
    _buttons->add(Login, "#8BBB56", "#6EA045");
    _buttons->get(Register)->setText(tr("Sign Up"));
    _buttons->get(Login)->setText(tr("Log In"));
    _buttons->get(Register)->setIcon(QIcon(PATH_RICON));
    _buttons->get(Login)->setIcon(QIcon(PATH_LICON));
    _buttons->get(Register)->setIconSize(QSize(16, 16));
    _buttons->get(Login)->setIconSize(QSize(16, 16));
    _buttons->get(Register)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Login)->setCursor(Qt::PointingHandCursor);
    connect(_buttons->get(Register), &QPushButton::clicked,
      this, &LoginWidget::signup);
    connect(_buttons->get(Login), &QPushButton::clicked,
      this, &LoginWidget::onLoginButtonClick);
    connect(&_encryptionWatcher, &QFutureWatcher<bool>::finished,
      this, &LoginWidget::onSessionStart);

    _helpBox = new QMessageBox(this);
    _helpBox->setIcon(QMessageBox::Question);
    _helpBox->setWindowTitle("Help");
    _helpBox->setText("Need help?");
    _helpBox->addButton("Forgot my password", QMessageBox::ActionRole);
    _helpBox->addButton("About", QMessageBox::ActionRole);
    _helpBox->addButton(QMessageBox::Cancel);
    _helpBox->setDefaultButton(QMessageBox::Cancel);
    connect(_helpBox, (void(QMessageBox::*)(QAbstractButton*))
      (&QMessageBox::buttonClicked), this, [=] (QAbstractButton* button) {
        if (button->text().contains("Forgot"))
            emit forget();
        else if (button->text().contains("About"))
            emit about();
    });

    _helpButton->settings().iconButton = true;
    _helpButton->setIcon(QIcon(PATH_HICON));
    _helpButton->setFixedSize(20,20);
    connect(_helpButton, &FlatButton::clicked, _helpBox, &QMessageBox::show);

    _loadingIndicator->setStyleSheet("background: transparent;");
    _loadingIndicator->setColor(palette().text().color());
    _loadingIndicator->setRoundness(50);
    _loadingIndicator->setMinimumTrailOpacity(5);
    _loadingIndicator->setTrailFadePercentage(100);
    _loadingIndicator->setRevolutionsPerSecond(2);
    _loadingIndicator->setNumberOfLines(12);
    _loadingIndicator->setLineLength(5);
    _loadingIndicator->setInnerRadius(4);
    _loadingIndicator->setLineWidth(2);

    _legalLabel->setText(QString("<p><b>© 2015 - 2019 %1 All Rights Reserved.</b></p>").arg(APP_CORP));
    _legalLabel->setAlignment(Qt::AlignHCenter);
}

void LoginWidget::lock()
{
    _bulkEdit->setDisabled(true);
    _buttons->setDisabled(true);
    _autologinSwitch->setDisabled(true);
    _helpButton->setDisabled(true);
    _loadingIndicator->start();
}

void LoginWidget::unlock()
{
    _bulkEdit->setEnabled(true);
    _buttons->setEnabled(true);
    _autologinSwitch->setEnabled(true);
    _helpButton->setEnabled(true);
    _loadingIndicator->stop();
}

void LoginWidget::clear()
{
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->clear();
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->clear();
}

void LoginWidget::onLoginButtonClick()
{
    auto email = static_cast<QLineEdit*>(_bulkEdit->get(Email))->text();
    auto password = static_cast<QLineEdit*>(_bulkEdit->get(Password))->text();

    if (email.isEmpty() || email.size() > 256 ||
        password.isEmpty() || password.size() > 256 ||
        !UtilityFunctions::isEmailFormatCorrect(email) ||
        !UtilityFunctions::isPasswordFormatCorrect(password)) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Fields cannot be either empty or incorrect.")
        );
        return;
    }

    lock();

    const auto& plan = AccountManager::login(email, password);
    bool succeed = !plan.isEmpty();

    if (!succeed) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Login is not successful, please check the information you provided.")
        );
    }

    unlock();

    if (succeed) {
        QTimer::singleShot(0, this, &LoginWidget::startSession);
        emit busy(tr("Decryption in progress"));
    }
}

void LoginWidget::startSession()
{
    const QString email = static_cast<QLineEdit*>(_bulkEdit->get(Email))->text();
    const QString password = static_cast<QLineEdit*>(_bulkEdit->get(Password))->text();

    UserManager::newUser(email);
    QFuture<bool> future = Async::run(QThreadPool::globalInstance(), &UserManager::start, email, password);

    _encryptionWatcher.setFuture(future);
}


void LoginWidget::onSessionStart()
{
    auto password = static_cast<QLineEdit*>(_bulkEdit->get(Password))->text();

    if (_encryptionWatcher.result()) {
        if (_autologinSwitch->isChecked())
            UserManager::setAutoLogin(password);
        else
            UserManager::clearAutoLogin();
    } else
        qFatal("Fatal : LoginWidget");

    QTimer::singleShot(500, this, &LoginWidget::clear);

    emit done();
}
