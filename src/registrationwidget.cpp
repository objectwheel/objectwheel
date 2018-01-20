#include <registrationwidget.h>
#include <fit.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <global.h>
#include <filemanager.h>
#include <authenticator.h>
#include <internetaccess.h>

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextStream>
#include <QMessageBox>

#define TERMS_HEIGHT     (fit::fx(35))
#define TERMS_WIDTH      (fit::fx(350))
#define SIZE_ICON        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_COUNTRIES   (":/resources/other/countries.txt")
#define PATH_ICON        (":/resources/images/register.png")
#define PATH_OICON       (":/resources/images/ok.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define pS               (QApplication::primaryScreen())

enum Fields { First, Last, Email, ConfirmEmail, Password, ConfirmPassword, Country, Company, Title, Phone };
enum Buttons { SignUp, Cancel };

static const QStringList& countries()
{
    static QStringList countries;

    if (countries.isEmpty()) {
        const auto& data = rdfile(PATH_COUNTRIES);

        QString country;
        QTextStream in(data);
        while (in.readLineInto(&country))
            countries << country.split("   ").first();
    }

    return countries;
}

RegistrationWidget::RegistrationWidget(QWidget *parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _iconLabel = new QLabel;
    _signupLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _termsWidget = new QWidget;
    _termsLayout = new QHBoxLayout(_termsWidget);
    _termsSwitch = new Switch;
    _termsLabel = new QLabel;
    _buttons = new ButtonSlice;
    _loadingIndicator = new WaitingSpinnerWidget(this, false, false);
    _legalLabel = new QLabel;

    _layout->setSpacing(fit::fx(12));
    _layout->addStretch();
    _layout->addWidget(_iconLabel);
    _layout->addWidget(_signupLabel);
    _layout->addWidget(_bulkEdit);
    _layout->addWidget(_termsWidget);
    _layout->addWidget(_buttons);
    _layout->addStretch();
    _layout->addWidget(_loadingIndicator);
    _layout->addStretch();
    _layout->addWidget(_legalLabel);

    _layout->setAlignment(_iconLabel, Qt::AlignCenter);
    _layout->setAlignment(_signupLabel, Qt::AlignCenter);
    _layout->setAlignment(_bulkEdit, Qt::AlignCenter);
    _layout->setAlignment(_termsWidget, Qt::AlignCenter);
    _layout->setAlignment(_buttons, Qt::AlignCenter);
    _layout->setAlignment(_loadingIndicator, Qt::AlignCenter);
    _layout->setAlignment(_legalLabel, Qt::AlignCenter);

    _termsLayout->setSpacing(fit::fx(5));
    _termsLayout->setContentsMargins(fit::fx(2.5), 0, 0, 0);
    _termsLayout->addWidget(_termsSwitch);
    _termsLayout->addWidget(_termsLabel);
    _termsLayout->setAlignment(_termsLabel, Qt::AlignVCenter);
    _termsLayout->setAlignment(_termsSwitch, Qt::AlignVCenter);
    _termsLayout->addStretch();

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

    _signupLabel->setFont(f);
    _signupLabel->setText(tr("Sign Up"));
    _signupLabel->setStyleSheet("color: #2E3A41");

    _bulkEdit->add(First, tr("First Name *"));
    _bulkEdit->add(Last, tr("Last Name *"));
    _bulkEdit->add(Email, tr("Email Address *"));
    _bulkEdit->add(ConfirmEmail, tr("Confirm Email *"));
    _bulkEdit->add(Password, tr("Password *"));
    _bulkEdit->add(ConfirmPassword, tr("Confirm Password *"));
    _bulkEdit->add(Country, tr("Country"), new QComboBox);
    _bulkEdit->add(Company, tr("Company"));
    _bulkEdit->add(Title, tr("Title"));
    _bulkEdit->add(Phone, tr("Phone"));
    _bulkEdit->setFixedWidth(TERMS_WIDTH);

    static_cast<QLineEdit*>(_bulkEdit->get(First))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Last))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmEmail))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Company))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Title))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Phone))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setEchoMode(QLineEdit::Password);
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->setEchoMode(QLineEdit::Password);

    auto cbox = static_cast<QComboBox*>(_bulkEdit->get(Country));
    cbox->setEditable(true);
    cbox->lineEdit()->setReadOnly(true);
    cbox->lineEdit()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cbox->setMaxVisibleItems(15);

    cbox->addItem(tr("Please select..."));
    for (int i = 0; i < countries().size(); i++)
        cbox->addItem(countries().at(i));

    cbox->setStyleSheet(
    "QComboBox {\
        border: none;\
        Background: transparent;\
        color: #2e3a41;\
    }\
    QComboBox::drop-down {\
        subcontrol-origin: padding;\
        subcontrol-position: top right;\
        border: none;\
    }\
    QComboBox::down-arrow {\
        image: url(:/resources/images/downarrow.png);\
    }\
    QComboBox QAbstractItemView {\
        Background: #f0f4f7;\
    }");

    _termsWidget->setObjectName("termsWidget");
    _termsWidget->setFixedSize(TERMS_WIDTH, TERMS_HEIGHT);
    _termsWidget->setStyleSheet(
        tr(
            "#termsWidget {"
            "    border-radius: %1;"
            "    Background: #12000000;"
            "    border: 1px solid #18000000;"
            "}"
        )
        .arg(int(TERMS_HEIGHT / 2.0))
    );

    _termsLabel->setFocusPolicy(Qt::NoFocus);
    _termsLabel->setTextFormat(Qt::RichText);
    _termsLabel->setText(tr(
        "I accept <a href=\"http://objectwheel.com/\" "
        "style=\"color: #2E3A41;\">the terms and conditions</a>"
    ));

    _termsLabel->setStyleSheet("color: #2E3A41");
    _termsLabel->setOpenExternalLinks(true);
    _termsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    _buttons->add(Cancel, "#cf5751", "#B34B46");
    _buttons->add(SignUp, "#6ab35f", "#599750");
    _buttons->get(SignUp)->setText(tr("Sign Up"));
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(SignUp)->setIcon(QIcon(PATH_OICON));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(SignUp)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = TERMS_WIDTH / 2.0;
    _buttons->triggerSettings();

    connect(_buttons->get(SignUp), SIGNAL(clicked(bool)), SLOT(onSignUpClicked()));
    connect(_buttons->get(Cancel), SIGNAL(clicked(bool)), SIGNAL(cancel()));

    _termsSwitch->settings().activeBackgroundColor = "#62A558";
    _termsSwitch->settings().activeBorderColor = "#538C4A";
    _termsSwitch->setChecked(false);
    _buttons->get(SignUp)->setDisabled(true);
    connect(_termsSwitch, SIGNAL(toggled(bool)),
      _buttons->get(SignUp), SLOT(setEnabled(bool)));

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

    _legalLabel->setText(TEXT_LEGAL);
    _legalLabel->setStyleSheet("color:#2E3A41;");
}

bool RegistrationWidget::checkEmail(const QString& email) const
{
    return email.contains(QRegExp("^[a-z0-9._%+-]+@[a-z0-9.-]+\\.[a-z]{2,4}$"));
}

bool RegistrationWidget::checkPassword(const QString& password) const
{
    return password.contains(QRegExp("^[><{}\\[\\]*!@\\-#$%^&+=~\\.\\,\\:a-zA-Z0-9]{6,25}$"));
}

void RegistrationWidget::clear()
{
    static_cast<QLineEdit*>(_bulkEdit->get(First))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(Last))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmEmail))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(Company))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(Title))->setText("");
    static_cast<QLineEdit*>(_bulkEdit->get(Phone))->setText("");
    static_cast<QComboBox*>(_bulkEdit->get(Country))->setCurrentIndex(0);
    _termsSwitch->setChecked(false);
    _buttons->setFocus();
}

void RegistrationWidget::lock()
{
    _bulkEdit->setDisabled(true);
    _termsLabel->setDisabled(true);
    _termsSwitch->setDisabled(true);
    _buttons->setDisabled(true);
    _loadingIndicator->start();
    _buttons->setFocus();
}

void RegistrationWidget::unlock()
{
    _bulkEdit->setEnabled(true);
    _termsLabel->setEnabled(true);
    _termsSwitch->setEnabled(true);
    _buttons->setEnabled(true);
    _loadingIndicator->stop();

    if (_termsSwitch->isChecked())
        _buttons->get(SignUp)->setEnabled(true);

    _buttons->setFocus();
}

void RegistrationWidget::onSignUpClicked()
{
    const auto& first = static_cast<QLineEdit*>(_bulkEdit->get(First))->text();
    const auto& last = static_cast<QLineEdit*>(_bulkEdit->get(Last))->text();
    const auto& email = static_cast<QLineEdit*>(_bulkEdit->get(Email))->text();
    const auto& cemail = static_cast<QLineEdit*>(_bulkEdit->get(ConfirmEmail))->text();
    const auto& password = static_cast<QLineEdit*>(_bulkEdit->get(Password))->text();
    const auto& cpassword = static_cast<QLineEdit*>(_bulkEdit->get(ConfirmPassword))->text();
    const auto& company = static_cast<QLineEdit*>(_bulkEdit->get(Company))->text();
    const auto& title = static_cast<QLineEdit*>(_bulkEdit->get(Title))->text();
    const auto& phone = static_cast<QLineEdit*>(_bulkEdit->get(Phone))->text();
    const auto& country = static_cast<QComboBox*>(_bulkEdit->get(Country))->currentText();

    if (first.isEmpty() || first.size() > 256 ||
        last.isEmpty() || last.size() > 256 ||
        email.isEmpty() || email.size() > 256 ||
        cemail.isEmpty() || cemail.size() > 256 ||
        password.isEmpty() || password.size() > 256 ||
        cpassword.isEmpty() || cpassword.size() > 256 ||
        country.size() > 256 || company.size() > 256 ||
        title.size() > 256 || phone.size() > 256
    ) {
        QMessageBox::warning(
            this,
            tr("Incorrect Information"),
            tr("Please make sure you have filled all the required fields "
               "with correct information. Fields can not exceed 256 characters.")
        );
        return;
    }

    if (email != cemail) {
        QMessageBox::warning(
            this,
            tr("Incorrect Email Addresses"),
            tr("Email addresses do not match.")
        );
        return;
    }

    if (password != cpassword) {
        QMessageBox::warning(
            this,
            tr("Incorrect Passwords"),
            tr("Passwords do not match.")
        );
        return;
    }

    if (!checkEmail(email)) {
        QMessageBox::warning(
            this,
            tr("Incorrect Email Address"),
            tr("Incorrect Email Address. Please check your email address.")
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
    Authenticator::instance()->signup(
        static_cast<QLineEdit*>(_bulkEdit->get(First))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Last))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Email))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Password))->text(),
        static_cast<QComboBox*>(_bulkEdit->get(Country))->currentText(),
        static_cast<QLineEdit*>(_bulkEdit->get(Company))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Title))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Phone))->text()
    );

    if (succeed)
        clear();
    else
        QMessageBox::warning(
            this,
            tr("Incorrect Information"),
            tr("Server rejected your request. Please review the information you entered. "
               "And make sure you are not trying to sign up more than once.")
        );

    unlock();

    if (succeed)
        emit done(email);
}
