#include <resetwidget.h>
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
#define PATH_ICON        (":/resources/images/lock.png")
#define PATH_VICON       (":/resources/images/ok.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define pS               (QApplication::primaryScreen())

enum Fields { Password, ConfirmPassword, Code };
enum Buttons { Apply, Cancel };

static bool resent = false;

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
    _loadingIndicator = new WaitingSpinnerWidget(this, false, false);

    _layout->setSpacing(fit::fx(12));
    _layout->addStretch();
    _layout->addWidget(_iconLabel);
    _layout->addWidget(_resetLabel);
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
    _layout->setAlignment(_resetLabel, Qt::AlignCenter);
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

    _resetLabel->setFont(f);
    _resetLabel->setText(tr("Email Reset"));
    _resetLabel->setStyleSheet("color: #2E3A41");

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

    _buttons->add(Cancel, "#5BC5F8", "#2592F9");
    _buttons->add(Apply, "#86CC63", "#75B257");
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(Apply)->setText(tr("Apply"));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Apply)->setIcon(QIcon(PATH_VICON));
    _buttons->get(Apply)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
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
    resent = false;
    unlock();
    _countdown->start(COUNTDOWN);
    _emailLabel->setText(tr("Please use the reset code\n"
        "we have sent to the following email address.\n") + email);
}

void ResetWidget::clear()
{
    static_cast<QLineEdit*>(_bulkEdit->get(Code))->clear();
}

void ResetWidget::lock()
{
    _bulkEdit->setDisabled(true);
    _buttons->get(Apply)->setDisabled(true);
    _buttons->get(Cancel)->setDisabled(true);
    _loadingIndicator->start();
}

void ResetWidget::unlock()
{
    _bulkEdit->setEnabled(true);
    _buttons->get(Apply)->setEnabled(true);
    _buttons->get(Cancel)->setEnabled(true);
    _loadingIndicator->stop();
}

void ResetWidget::onCancelClicked()
{
    _countdown->stop();
    clear();
    emit cancel();
}

void ResetWidget::onApplyClicked()
{
    auto email = _emailLabel->text().split("\n").at(2);
    auto code = static_cast<QLineEdit*>(_bulkEdit->get(Code))->text();

    if (code.isEmpty() || code.size() != 6) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Reset code is incorrect.")
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

    if (succeed) {
        clear();
        _countdown->stop();
    } else {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Server rejected your code. Or, you might exceed "
               "the reset trial limit. Try again later.")
        );
    }

    unlock();

    if (succeed)
        emit done();
}
