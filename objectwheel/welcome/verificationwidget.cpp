#include <verificationwidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <countdown.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QRegExpValidator>

#define COUNTDOWN        300
#define BUTTONS_WIDTH    (300)
#define PATH_VICON       (":/images/welcome/ok.png")
#define PATH_RICON       (":/images/welcome/reset.png")
#define PATH_CICON       (":/images/welcome/cancel.png")

enum Fields { Code };
enum Buttons { Verify, Resend, Cancel };

static bool resent = false;

VerificationWidget::VerificationWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _countdownLabel = new QLabel;
    _countdown = new Countdown;
    _verificationLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _emailLabel = new QLabel;
    _buttons = new ButtonSlice;
    _loadingIndicator = new WaitingSpinnerWidget(this, false);
    // TODO: Check out if the given email address is valid before sending request to the server
    _layout->setSpacing(6);
    _layout->addStretch();
    _layout->addWidget(_iconLabel);
    _layout->addWidget(_verificationLabel);
    _layout->addSpacing(40);
    _layout->addWidget(_countdownLabel);
    _layout->addWidget(_countdown);
    _layout->addSpacing(40);
    _layout->addWidget(_emailLabel);
    _layout->addWidget(_bulkEdit);
    _layout->addWidget(_buttons);
    _layout->addStretch();
    _layout->addWidget(_loadingIndicator);
    _layout->addStretch();

    _layout->setAlignment(_iconLabel, Qt::AlignCenter);
    _layout->setAlignment(_countdownLabel, Qt::AlignCenter);
    _layout->setAlignment(_countdown, Qt::AlignCenter);
    _layout->setAlignment(_verificationLabel, Qt::AlignCenter);
    _layout->setAlignment(_emailLabel, Qt::AlignCenter);
    _layout->setAlignment(_bulkEdit, Qt::AlignCenter);
    _layout->setAlignment(_buttons, Qt::AlignCenter);
    _layout->setAlignment(_loadingIndicator, Qt::AlignCenter);

    _iconLabel->setFixedSize(QSize(60, 60));
    _iconLabel->setPixmap(PaintUtils::pixmap(":/images/welcome/verification.svg", QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    _verificationLabel->setFont(f);
    _verificationLabel->setText(tr("Email Verification"));

    _countdownLabel->setAlignment(Qt::AlignHCenter);
    _countdownLabel->setText(tr("You have left:"));
    _countdownLabel->setStyleSheet("color: #70000000");

    _emailLabel->setAlignment(Qt::AlignHCenter);
    _emailLabel->setStyleSheet("color: #70000000");

    _bulkEdit->add(Code, tr("Enter Verification Code"));
    _bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    _bulkEdit->get<QLineEdit*>(Code)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Code)->setValidator(new QRegExpValidator(QRegExp("^\\d{1,6}$"), this));

    _buttons->add(Cancel, "#CC5D67", "#B2525A");
    _buttons->add(Resend, "#5BC5F8", "#2592F9");
    _buttons->add(Verify, "#86CC63", "#75B257");
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(Resend)->setText(tr("Resend"));
    _buttons->get(Verify)->setText(tr("Verify"));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Resend)->setIcon(QIcon(PATH_RICON));
    _buttons->get(Verify)->setIcon(QIcon(PATH_VICON));
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Resend)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Verify)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 3.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Verify), &QPushButton::clicked, this, &VerificationWidget::onVerifyClicked);
    connect(_buttons->get(Resend), &QPushButton::clicked, this, &VerificationWidget::onResendClicked);
    connect(_buttons->get(Cancel), &QPushButton::clicked, this, &VerificationWidget::onCancelClicked);

    connect(_countdown, &Countdown::finished, [=]{
        UtilityFunctions::showMessage(
                    this, tr("Expired"),
                    tr("Your verification code has been expired, please try again later."));
    });

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
}

void VerificationWidget::setEmail(const QString& email)
{
    resent = false;
    unlock();
    _countdown->start(COUNTDOWN);
    _emailLabel->setText(tr("Please use the verification code\n"
        "we have sent to the following email address.\n") + email);
}

void VerificationWidget::clear()
{
    _countdown->stop();
    _bulkEdit->get<QLineEdit*>(Code)->clear();
}

void VerificationWidget::lock()
{
    _bulkEdit->setDisabled(true);
    _buttons->get(Verify)->setDisabled(true);
    _buttons->get(Resend)->setDisabled(true);
    _buttons->get(Cancel)->setDisabled(true);
    _loadingIndicator->start();
}

void VerificationWidget::unlock()
{
    _bulkEdit->setEnabled(true);
    _buttons->get(Verify)->setEnabled(true);
    if (!resent)
        _buttons->get(Resend)->setEnabled(true);
    _buttons->get(Cancel)->setEnabled(true);
    _loadingIndicator->stop();
}

void VerificationWidget::onCancelClicked()
{
    clear();
    emit cancel();
}

void VerificationWidget::onResendClicked()
{
    auto email = _emailLabel->text().split("\n").at(2);

    lock();

//    bool succeed = RegistrationApiManager::resend(email);

//    if (succeed) {
//        clear();
//        _countdown->start();

//        QMessageBox::information(
//            this,
//            tr("Resend succeed"),
//            tr("New verification code has been sent.")
//        );
//    } else {
//        QMessageBox::warning(
//            this,
//            tr("Oops"),
//            tr("Server rejected your request. You might exceed "
//               "the verification trial limit. Try again later.")
//        );
//    }

//    resent = true;
//    unlock();
}

void VerificationWidget::onVerifyClicked()
{
    auto email = _emailLabel->text().split("\n").at(2);
    auto code = _bulkEdit->get<QLineEdit*>(Code)->text();

    if (code.isEmpty() || code.size() != 6) {
        UtilityFunctions::showMessage(this, tr("Incorrect information"),
                                      tr("Verification code is incorrect."));
        return;
    }

    lock();

//    bool succeed = RegistrationApiManager::verify(email, code);

//    if (succeed)
//        clear();
//    else {
//        QMessageBox::warning(
//            this,
//            tr("Oops"),
//            tr("Server rejected your code. Or, you might exceed "
//               "the verification trial limit. Try again later.")
//        );
//    }

//    unlock();

//    if (succeed)
//        emit done();
}
