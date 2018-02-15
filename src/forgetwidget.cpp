#include <forgetwidget.h>
#include <fit.h>
#include <buttonslice.h>
#include <bulkedit.h>
#include <internetaccess.h>
#include <waitingspinnerwidget.h>
#include <authenticator.h>

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>

#define BUTTONS_WIDTH    (fit::fx(350))
#define SIZE_ICON        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_ICON        (":/resources/images/forgot.png")
#define PATH_NICON       (":/resources/images/load.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define pS               (QApplication::primaryScreen())

enum Fields { Email };
enum Buttons { Next, Back };

static bool checkEmail(const QString& email)
{
    return email.contains(QRegExp("^[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,4}$"));
}

ForgetWidget::ForgetWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _forgotLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _buttons = new ButtonSlice;
    _loadingIndicator = new WaitingSpinnerWidget(this, false);

    _layout->setSpacing(fit::fx(12));
    _layout->addStretch();
    _layout->addWidget(_iconLabel, 0 , Qt::AlignCenter);
    _layout->addSpacing(fit::fx(10));
    _layout->addWidget(_forgotLabel, 0 , Qt::AlignCenter);
    _layout->addSpacing(fit::fx(50));
    _layout->addWidget(_bulkEdit, 0 , Qt::AlignCenter);
    _layout->addSpacing(fit::fx(10));
    _layout->addWidget(_buttons, 0 , Qt::AlignCenter);
    _layout->addStretch();
    _layout->addWidget(_loadingIndicator, 0 , Qt::AlignCenter);
    _layout->addStretch();

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

    _forgotLabel->setFont(f);
    _forgotLabel->setText(tr("Password Reset"));
    _forgotLabel->setStyleSheet("color: #2E3A41");

    _bulkEdit->add(Email, tr("Email Address"));
    _bulkEdit->setFixedWidth(BUTTONS_WIDTH);

    static_cast<QLineEdit*>(_bulkEdit->get(Email))
      ->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    _buttons->add(Back, "#5BC5F8", "#2592F9");
    _buttons->add(Next, "#8BBB56", "#6EA045");
    _buttons->get(Next)->setText(tr("Next"));
    _buttons->get(Back)->setText(tr("Back"));
    _buttons->get(Next)->setIcon(QIcon(PATH_NICON));
    _buttons->get(Back)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 2.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Back), SIGNAL(clicked(bool)), SIGNAL(back()));
    connect(_buttons->get(Next), SIGNAL(clicked(bool)), SLOT(onNextClicked()));

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

void ForgetWidget::clear()
{
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->setText("");
}

void ForgetWidget::lock()
{
    _bulkEdit->setDisabled(true);
    _buttons->setDisabled(true);
    _loadingIndicator->start();
}

void ForgetWidget::unlock()
{
    _bulkEdit->setEnabled(true);
    _buttons->setEnabled(true);
    _loadingIndicator->stop();
}

void ForgetWidget::onNextClicked()
{
    const auto& email = static_cast<QLineEdit*>(_bulkEdit->get(Email))->text();

    if (email.isEmpty() || email.size() > 256 ||
        !checkEmail(email)
        ) {
        QMessageBox::warning(
                    this,
                    tr("Oops"),
                    tr("Incorrect Email Address. Please check your email address.")
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

    bool succeed =
    Authenticator::instance()->forget(
        static_cast<QLineEdit*>(_bulkEdit->get(Email))->text()
    );

    if (succeed)
        clear();
    else
        QMessageBox::warning(
            this,
            tr("Incorrect Information"),
            tr("Server rejected your request. Possibly incorrect email address.")
        );

    unlock();

    if (succeed)
        emit done(email);
}