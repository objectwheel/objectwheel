#include <registrationwidget.h>
#include <coreconstants.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <utilityfunctions.h>
#include <paintutils.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QTextStream>
#include <QMessageBox>
#include <QFile>

#define TERMS_HEIGHT     (35)
#define TERMS_WIDTH      (350)
#define PATH_COUNTRIES   (":/other/countries.txt")
#define PATH_OICON       (":/images/welcome/load.png")
#define PATH_BICON       (":/images/welcome/unload.png")

enum Fields { First, Last, Email, ConfirmEmail, Password, ConfirmPassword, Country, Company, Title, Phone };
enum Buttons { Next, Back };

static const QStringList& countries()
{
    static QStringList countries;

    if (countries.isEmpty()) {
        QFile file(PATH_COUNTRIES);
        if (!file.open(QFile::ReadOnly)) {
            qWarning("countries: Cannot read file");
            return countries;
        }

        QString country;
        QTextStream in(&file);
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

    _iconLabel->setFixedSize(QSize(60, 60));
    _iconLabel->setPixmap(PaintUtils::pixmap(":/images/welcome/register.svg", QSize(60, 60), this));

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

    _bulkEdit->get<QLineEdit*>(First)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Last)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(ConfirmEmail)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Password)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(ConfirmPassword)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Company)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Title)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Phone)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    _bulkEdit->get<QLineEdit*>(Password)->setEchoMode(QLineEdit::Password);
    _bulkEdit->get<QLineEdit*>(ConfirmPassword)->setEchoMode(QLineEdit::Password);

    auto cbox = _bulkEdit->get<QComboBox*>(Country);
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
        image: url(:/images/welcome/downarrow.png);\
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
        "I accept <a href=\"%1\">the terms and conditions</a>"
    ).arg(CoreConstants::TAC_ADDRESS));

    _termsLabel->setOpenExternalLinks(true);
    _termsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);

    _buttons->add(Back, "#5BC5F8", "#2592F9");
    _buttons->add(Next, "#8BBB56", "#6EA045");
    _buttons->get(Next)->setText(tr("Next"));
    _buttons->get(Back)->setText(tr("Back"));
    _buttons->get(Next)->setIcon(QIcon(PATH_OICON));
    _buttons->get(Back)->setIcon(QIcon(PATH_BICON));
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
    _bulkEdit->get<QLineEdit*>(First)->setText("");
    _bulkEdit->get<QLineEdit*>(Last)->setText("");
    _bulkEdit->get<QLineEdit*>(Email)->setText("");
    _bulkEdit->get<QLineEdit*>(ConfirmEmail)->setText("");
    _bulkEdit->get<QLineEdit*>(Password)->setText("");
    _bulkEdit->get<QLineEdit*>(ConfirmPassword)->setText("");
    _bulkEdit->get<QLineEdit*>(Company)->setText("");
    _bulkEdit->get<QLineEdit*>(Title)->setText("");
    _bulkEdit->get<QLineEdit*>(Phone)->setText("");
    _bulkEdit->get<QComboBox*>(Country)->setCurrentIndex(0);
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
    const auto& first = _bulkEdit->get<QLineEdit*>(First)->text();
    const auto& last = _bulkEdit->get<QLineEdit*>(Last)->text();
    const auto& email = _bulkEdit->get<QLineEdit*>(Email)->text();
    const auto& cemail = _bulkEdit->get<QLineEdit*>(ConfirmEmail)->text();
    const auto& password = _bulkEdit->get<QLineEdit*>(Password)->text();
    const auto& cpassword = _bulkEdit->get<QLineEdit*>(ConfirmPassword)->text();
    const auto& company = _bulkEdit->get<QLineEdit*>(Company)->text();
    const auto& title = _bulkEdit->get<QLineEdit*>(Title)->text();
    const auto& phone = _bulkEdit->get<QLineEdit*>(Phone)->text();
    const auto& country = _bulkEdit->get<QComboBox*>(Country)->currentText();

    if (!_termsSwitch->isChecked()) {
        UtilityFunctions::showMessage(
                    this, tr("Oops"),
                    tr("Please accept the terms and conditions first in "
                       "order to continue the registration process."));
        return;
    }

    if (first.isEmpty() || first.size() > 255 ||
        last.isEmpty() || last.size() > 255 ||
        email.isEmpty() || email.size() > 255 ||
        cemail.isEmpty() || cemail.size() > 255 ||
        password.isEmpty() || password.size() > 255 ||
        cpassword.isEmpty() || cpassword.size() > 255 ||
        country.size() > 255 || company.size() > 255 ||
        title.size() > 255 || phone.size() > 255
    ) {
        UtilityFunctions::showMessage(
                    this, tr("Incorrect information"),
                    tr("Please make sure you have filled all the required fields "
                       "with necessary information. Fields cannot exceed 255 characters long."));
        return;
    }

    if (email != cemail) {
        UtilityFunctions::showMessage(
                    this, tr("Incorrect email addresses"),
                    tr("Email addresses you entered do not match."));
        return;
    }

    if (password != cpassword) {
        UtilityFunctions::showMessage(
                    this, tr("Incorrect passwords"),
                    tr("Passwords you entered do not match."));
        return;
    }

    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(
                    this, tr("Incorrect email address"),
                    tr("Broken email address, please checkout the email address you entered."));
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password)) {
        UtilityFunctions::showMessage(
                    this, tr("Incorrect password"),
                    tr("Incorrect password, your password must be in between "
                       "6 and 35 characters long. Also please checkout if it contains invalid characters."));
        return;
    }

    lock();

//    bool succeed =
//    RegistrationApiManager::signup(
//        _bulkEdit->get<QLineEdit*>(First)->text(),
//        _bulkEdit->get<QLineEdit*>(Last)->text(),
//        _bulkEdit->get<QLineEdit*>(Email)->text(),
//        _bulkEdit->get<QLineEdit*>(Password)->text(),
//        static_cast<QComboBox*>(_bulkEdit->get(Country)->currentText() != "Please select..." ?
//        static_cast<QComboBox*>(_bulkEdit->get(Country)->currentText() : "",
//        _bulkEdit->get<QLineEdit*>(Company)->text(),
//        _bulkEdit->get<QLineEdit*>(Title)->text(),
//        _bulkEdit->get<QLineEdit*>(Phone)->text()
//    );

//    if (succeed)
//        clear();
//    else
//        QMessageBox::warning(
//            this,
//            tr("Incorrect Information"),
//            tr("Server rejected your request. Please review the information you entered. "
//               "And make sure you are not trying to sign up more than once.")
//        );

//    unlock();

//    if (succeed)
//        emit done(email);
}
