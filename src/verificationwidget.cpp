#include <verificationwidget.h>
#include <fit.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <authenticator.h>
#include <internetaccess.h>
#include <countdown.h>

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QRegExpValidator>

#define COUNTDOWN        300
#define BUTTONS_WIDTH    (fit::fx(300))
#define SIZE_ICON        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_ICON        (":/resources/images/verification.png")
#define PATH_VICON       (":/resources/images/ok.png")
#define PATH_RICON       (":/resources/images/reset.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define pS               (QApplication::primaryScreen())

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

    _layout->setSpacing(fit::fx(12));
    _layout->addStretch();
    _layout->addWidget(_iconLabel);
    _layout->addWidget(_verificationLabel);
    _layout->addSpacing(fit::fx(40));
    _layout->addWidget(_countdownLabel);
    _layout->addWidget(_countdown);
    _layout->addSpacing(fit::fx(40));
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

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(pS->devicePixelRatio());

    _iconLabel->setFixedSize(SIZE_ICON);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));

    _verificationLabel->setFont(f);
    _verificationLabel->setText(tr("Email Verification"));
    _verificationLabel->setStyleSheet("color: #2E3A41");

    _countdownLabel->setAlignment(Qt::AlignHCenter);
    _countdownLabel->setText(tr("You have left:"));
    _countdownLabel->setStyleSheet("color: #70000000");

    _emailLabel->setAlignment(Qt::AlignHCenter);
    _emailLabel->setStyleSheet("color: #70000000");

    _bulkEdit->add(Code, tr("Enter Verification Code"));
    _bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    static_cast<QLineEdit*>(_bulkEdit->get(Code))
      ->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Code))
      ->setValidator(new QRegExpValidator(QRegExp("^\\d{1,6}$"), this));

    _buttons->add(Cancel, "#CC5D67", "#B2525A");
    _buttons->add(Resend, "#5BC5F8", "#2592F9");
    _buttons->add(Verify, "#86CC63", "#75B257");
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(Resend)->setText(tr("Resend"));
    _buttons->get(Verify)->setText(tr("Verify"));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Resend)->setIcon(QIcon(PATH_RICON));
    _buttons->get(Verify)->setIcon(QIcon(PATH_VICON));
    _buttons->get(Verify)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Resend)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 3.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Verify), SIGNAL(clicked(bool)), SLOT(onVerifyClicked()));
    connect(_buttons->get(Resend), SIGNAL(clicked(bool)), SLOT(onResendClicked()));
    connect(_buttons->get(Cancel), SIGNAL(clicked(bool)), SLOT(onCancelClicked()));

    connect(_countdown, &Countdown::finished, [=]{
        QMessageBox::warning(
            this,
            tr("Expired"),
            tr("Your verification code has been expired. Try resending it.")
        );
    });

    _loadingIndicator->setStyleSheet("Background: transparent;");
    _loadingIndicator->setColor("#2E3A41");
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
    static_cast<QLineEdit*>(_bulkEdit->get(Code))->clear();
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

    if (!InternetAccess::available()) {
        QMessageBox::warning(
            this,
            tr("No Internet Access"),
            tr("Unable to connect to server. Check your internet connection.")
        );

        unlock();
        return;
    }

    bool succeed = Authenticator::instance()->resend(email);

    if (succeed) {
        clear();
        _countdown->start();

        QMessageBox::information(
            this,
            tr("Resend succeed"),
            tr("New verification code has been sent.")
        );
    } else {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Server rejected your request. You might exceed "
               "the verification trial limit. Try again later.")
        );
    }

    resent = true;
    unlock();
}

void VerificationWidget::onVerifyClicked()
{
    auto email = _emailLabel->text().split("\n").at(2);
    auto code = static_cast<QLineEdit*>(_bulkEdit->get(Code))->text();

    if (code.isEmpty() || code.size() != 6) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Verification code is incorrect.")
        );
        return;
    }

    lock();

    if (!InternetAccess::available()) {
        QMessageBox::warning(
            this,
            tr("No Internet Access"),
            tr("Unable to connect to server. Check your internet connection.")
        );

        unlock();
        return;
    }

    bool succeed = Authenticator::instance()->verify(email, code);

    if (succeed)
        clear();
    else {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Server rejected your code. Or, you might exceed "
               "the verification trial limit. Try again later.")
        );
    }

    unlock();

    if (succeed)
        emit done();
}