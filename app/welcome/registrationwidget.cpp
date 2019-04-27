#include <registrationwidget.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <accountmanager.h>
#include <utilityfunctions.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextStream>
#include <QMessageBox>

#define TERMS_HEIGHT     (35)
#define TERMS_WIDTH      (350)
#define SIZE_ICON        (QSize(48, 48))
#define PATH_COUNTRIES   (":/resources/other/countries.txt")
#define PATH_ICON        (":/images/register.png")
#define PATH_OICON       (":/images/load.png")
#define PATH_BICON       (":/images/unload.png")

enum Fields { First, Last, Email, ConfirmEmail, Password, ConfirmPassword, Country, Company, Title, Phone };
enum Buttons { Next, Back };

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
    _loadingIndicator = new WaitingSpinnerWidget(this, false);

    _layout->setSpacing(6);

    _layout->addStretch();
    _layout->addWidget(_iconLabel,0 , Qt::AlignCenter);
    _layout->addWidget(_signupLabel,0 , Qt::AlignCenter);
    _layout->addWidget(_bulkEdit, 0, Qt::AlignCenter);
    _layout->addWidget(_termsWidget, 0, Qt::AlignCenter);
    _layout->addWidget(_buttons, 0, Qt::AlignCenter);
    _layout->addStretch();
    _layout->addWidget(_loadingIndicator, 0, Qt::AlignCenter);
    _layout->addStretch();

    _termsLayout->setSpacing(5);
    _termsLayout->setContentsMargins(2, 0, 0, 0);
    _termsLayout->addWidget(_termsSwitch);
    _termsLayout->addWidget(_termsLabel);
    _termsLayout->setAlignment(_termsLabel, Qt::AlignVCenter);
    _termsLayout->setAlignment(_termsSwitch, Qt::AlignVCenter);
    _termsLayout->addStretch();

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(devicePixelRatioF());

    _iconLabel->setFixedSize(SIZE_ICON);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * devicePixelRatioF(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    _signupLabel->setFont(f);
    _signupLabel->setText(tr("Sign Up"));

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
        background: transparent;\
    }\
    QComboBox::drop-down {\
        subcontrol-origin: padding;\
        subcontrol-position: top right;\
        border: none;\
    }\
    QComboBox::down-arrow {\
        image: url(:/images/downarrow.png);\
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
        "I accept <a href=\"http://objectwheel.com/\">the terms and conditions</a>"
    ));

    _termsLabel->setOpenExternalLinks(true);
    _termsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    _buttons->add(Back, "#5BC5F8", "#2592F9");
    _buttons->add(Next, "#8BBB56", "#6EA045");
    _buttons->get(Next)->setText(tr("Next"));
    _buttons->get(Back)->setText(tr("Back"));
    _buttons->get(Next)->setIcon(QIcon(PATH_OICON));
    _buttons->get(Back)->setIcon(QIcon(PATH_BICON));
    _buttons->get(Next)->setIconSize(QSize(16, 16));
    _buttons->get(Back)->setIconSize(QSize(16, 16));
    _buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = TERMS_WIDTH / 2.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Next), &QPushButton::clicked, this, &RegistrationWidget::onNextClicked);
    connect(_buttons->get(Back), &QPushButton::clicked, this, &RegistrationWidget::back);

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
        _buttons->get(Next)->setEnabled(true);

    _buttons->setFocus();
}

void RegistrationWidget::onNextClicked()
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

    if (!_termsSwitch->isChecked()) {
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Please accept the terms and conditions first.")
        );
        return;
    }

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

    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        QMessageBox::warning(
            this,
            tr("Incorrect Email Address"),
            tr("Incorrect Email Address. Please check your email address.")
        );
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password)) {
        QMessageBox::warning(
            this,
            tr("Incorrect Password"),
            tr("Incorrect Password. Your password must be in between "
               "6 and 35 characters long. Also please check it if contains invalid characters.")
        );
        return;
    }

    lock();

    bool succeed =
    AccountManager::signup(
        static_cast<QLineEdit*>(_bulkEdit->get(First))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Last))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Email))->text(),
        static_cast<QLineEdit*>(_bulkEdit->get(Password))->text(),
        static_cast<QComboBox*>(_bulkEdit->get(Country))->currentText() != "Please select..." ?
        static_cast<QComboBox*>(_bulkEdit->get(Country))->currentText() : "",
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
