#include <resetwidget.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <authenticator.h>
#include <countdown.h>
#include <dpr.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QRegExpValidator>

#define COUNTDOWN        300
#define BUTTONS_WIDTH    (300)
#define SIZE_ICON        (QSize(80, 80))
#define PATH_ICON        (":/resources/images/lock.png")
#define PATH_VICON       (":/resources/images/ok.png")
#define PATH_CICON       (":/resources/images/cancel.png")

static bool checkPassword(const QString& password)
{
    return password.contains(QRegExp("^[><{}\\[\\]*!@\\-#$%^&+=~\\.\\,\\:a-zA-Z0-9]{6,25}$"));
}

enum Fields { Password, ConfirmPassword, Code };
enum Buttons { Apply, Cancel };

ResetWidget::ResetWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _countdownLabel = new QLabel;
    _countdown = new Countdown;
    _resetLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _emailLabel = new QLabel;
    _buttons = new ButtonSlice;
    _loadingIndicator = new WaitingSpinnerWidget(this, false);

    _layout->setSpacing(12);
    _layout->addStretch();
    _layout->addWidget(_iconLabel);
    _layout->addWidget(_resetLabel);
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
    _layout->setAlignment(_resetLabel, Qt::AlignCenter);
    _layout->setAlignment(_emailLabel, Qt::AlignCenter);
    _layout->setAlignment(_bulkEdit, Qt::AlignCenter);
    _layout->setAlignment(_buttons, Qt::AlignCenter);
    _layout->setAlignment(_loadingIndicator, Qt::AlignCenter);

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(DPR);

    _iconLabel->setFixedSize(SIZE_ICON);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * DPR,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(18);

    _resetLabel->setFont(f);
    _resetLabel->setText(tr("Verify Password Reset"));
    _resetLabel->setStyleSheet("color: black");

    _countdownLabel->setAlignment(Qt::AlignHCenter);
    _countdownLabel->setText(tr("You have left:"));
    _countdownLabel->setStyleSheet("color: #70000000");

    _emailLabel->setAlignment(Qt::AlignHCenter);
    _emailLabel->setStyleSheet("color: #70000000");

    _bulkEdit->add(Password, tr("New Password"));
    _bulkEdit->add(ConfirmPassword, tr("Confirm Password"));
    _bulkEdit->add(Code, tr("Reset Code"));
    _bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    static_cast<QLineEdit*>(_bulkEdit->get(Code))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Code))->setValidator(new QRegExpValidator(QRegExp("^\\d{1,6}$"), this));
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setEchoMode(QLineEdit::Password);
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->setEchoMode(QLineEdit::Password);

    _buttons->add(Cancel, "#CC5D67", "#B2525A");
    _buttons->add(Apply, "#86CC63", "#75B257");
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(Apply)->setText(tr("Apply"));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Apply)->setIcon(QIcon(PATH_VICON));
    _buttons->get(Cancel)->setIconSize(QSize(16, 16));
    _buttons->get(Apply)->setIconSize(QSize(16, 16));
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Apply)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 2.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Apply), SIGNAL(clicked(bool)), SLOT(onApplyClicked()));
    connect(_buttons->get(Cancel), SIGNAL(clicked(bool)), SLOT(onCancelClicked()));

    connect(_countdown, &Countdown::finished, [=]{
        QMessageBox::warning(
            this,
            tr("Expired"),
            tr("Your reset code has been expired. Try again later.")
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

void ResetWidget::setEmail(const QString& email)
{
    _countdown->start(COUNTDOWN);
    _emailLabel->setText(tr("Please use the reset code we have sent\n"
      "to the following email address, to reset your password.\n") + email);
}

void ResetWidget::clear()
{
    _countdown->stop();
    static_cast<QLineEdit*>(_bulkEdit->get(Code))->clear();
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->clear();
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->clear();
}

void ResetWidget::lock()
{
    _bulkEdit->setDisabled(true);
    _buttons->setDisabled(true);
    _loadingIndicator->start();
}

void ResetWidget::unlock()
{
    _bulkEdit->setEnabled(true);
    _buttons->setEnabled(true);
    _loadingIndicator->stop();
}

void ResetWidget::onCancelClicked()
{
    clear();
    emit cancel();
}

void ResetWidget::onApplyClicked()
{
    auto email = _emailLabel->text().split("\n").at(2);
    const auto& code = static_cast<QLineEdit*>(_bulkEdit->get(Code))->text();
    const auto& password = static_cast<QLineEdit*>(_bulkEdit->get(Password))->text();
    const auto& cpassword = static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->text();

    if (password != cpassword) {
        QMessageBox::warning(
            this,
            tr("Incorrect Passwords"),
            tr("Passwords do not match.")
        );
        return;
    }

    if (!checkPassword(password)) {
        QMessageBox::warning(
            this,
            tr("Incorrect Password"),
            tr("Incorrect Password. Your password must be in between "
               "6 and 25 characters long. Also please check it if contains invalid characters.")
        );
        return;
    }

    if (code.isEmpty() || code.size() != 6) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Reset code is incorrect.")
        );
        return;
    }

    lock();

    bool succeed = Authenticator::instance()->reset(email, password, code);

    if (succeed)
        clear();
    else {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Server rejected your code. You might mistyped "
               "the reset code. Try again.")
        );
    }

    unlock();

    if (succeed)
        emit done();
}
