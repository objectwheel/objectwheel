#include <registrationwidget.h>
#include <appconstants.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <registrationapimanager.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <utilityfunctions.h>

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>

enum Fields { First, Last, Email, ConfirmEmail, Password, ConfirmPassword, Country, Company, Title, Phone };
enum Buttons { Next, Back };

RegistrationWidget::RegistrationWidget(QWidget* parent) : QWidget(parent)
  , m_bulkEdit(new BulkEdit(this))
  , m_termsSwitch(new Switch(this))
  , m_buttons(new ButtonSlice(this))
  , m_loadingIndicator(new WaitingSpinnerWidget(this, false))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(":/images/welcome/register.svg", QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto signupLabel = new QLabel(this);
    signupLabel->setFont(f);
    signupLabel->setText(tr("Sign Up"));

    m_bulkEdit->add(First, tr("First Name *"));
    m_bulkEdit->add(Last, tr("Last Name *"));
    m_bulkEdit->add(Email, tr("Email Address *"));
    m_bulkEdit->add(ConfirmEmail, tr("Confirm Email *"));
    m_bulkEdit->add(Password, tr("Password *"));
    m_bulkEdit->add(ConfirmPassword, tr("Confirm Password *"));
    m_bulkEdit->add(Country, tr("Country"), new QComboBox(this));
    m_bulkEdit->add(Company, tr("Company"));
    m_bulkEdit->add(Title, tr("Title"));
    m_bulkEdit->add(Phone, tr("Phone"));
    m_bulkEdit->setFixedWidth(350);

    m_bulkEdit->get<QLineEdit*>(First)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Last)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(ConfirmEmail)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Password)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Company)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Title)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Phone)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Password)->setEchoMode(QLineEdit::Password);
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->setEchoMode(QLineEdit::Password);

    auto termsWidget = new QWidget(this);
    termsWidget->setFixedSize(m_bulkEdit->width(), 35);
    termsWidget->setObjectName("termsWidget");
    termsWidget->setStyleSheet(tr("#termsWidget {"
                                  "    border-radius: %1;"
                                  "    Background: #12000000;"
                                  "    border: 1px solid #18000000;"
                                  "}").arg(int(termsWidget->height() / 2.0)));

    auto termsLabel = new QLabel(termsWidget);
    termsLabel->setFocusPolicy(Qt::NoFocus);
    termsLabel->setTextFormat(Qt::RichText);
    termsLabel->setOpenExternalLinks(true);
    termsLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    termsLabel->setText(tr("I accept <a href=\"%1\">the terms and conditions</a>").arg(AppConstants::TAC_URL));

    auto termsLayout = new QHBoxLayout(termsWidget);
    termsLayout->setSpacing(5);
    termsLayout->setContentsMargins(2, 0, 0, 0);
    termsLayout->addWidget(m_termsSwitch, 0, Qt::AlignVCenter);
    termsLayout->addWidget(termsLabel, 0, Qt::AlignVCenter);
    termsLayout->addStretch();

    m_buttons->add(Back, "#5BC5F8", "#2592F9");
    m_buttons->add(Next, "#8BBB56", "#6EA045");
    m_buttons->get(Next)->setText(tr("Next"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Next)->setIcon(QIcon(":/images/welcome/load.png"));
    m_buttons->get(Back)->setIcon(QIcon(":/images/welcome/unload.png"));
    m_buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = m_bulkEdit->width() / 2.0;
    m_buttons->triggerSettings();

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

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(signupLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(termsWidget, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_loadingIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

    auto cbox = m_bulkEdit->get<QComboBox*>(Country);
    cbox->setEditable(true);
    cbox->lineEdit()->setReadOnly(true);
    cbox->lineEdit()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    cbox->setMaxVisibleItems(15);

    cbox->addItem(tr("Please select..."));
    cbox->addItems(UtilityFunctions::countryList());
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

    connect(m_buttons->get(Next), &QPushButton::clicked,
            this, &RegistrationWidget::onNextClicked);
    connect(m_buttons->get(Back), &QPushButton::clicked,
            this, &RegistrationWidget::back);
}

void RegistrationWidget::clear()
{
    m_bulkEdit->get<QLineEdit*>(First)->setText("");
    m_bulkEdit->get<QLineEdit*>(Last)->setText("");
    m_bulkEdit->get<QLineEdit*>(Email)->setText("");
    m_bulkEdit->get<QLineEdit*>(ConfirmEmail)->setText("");
    m_bulkEdit->get<QLineEdit*>(Password)->setText("");
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->setText("");
    m_bulkEdit->get<QLineEdit*>(Company)->setText("");
    m_bulkEdit->get<QLineEdit*>(Title)->setText("");
    m_bulkEdit->get<QLineEdit*>(Phone)->setText("");
    m_bulkEdit->get<QComboBox*>(Country)->setCurrentIndex(0);
    m_termsSwitch->setChecked(false);
    m_buttons->setFocus();
}

void RegistrationWidget::lock()
{
    m_loadingIndicator->start();
    m_buttons->setFocus();
}

void RegistrationWidget::unlock()
{
    m_loadingIndicator->stop();
    m_buttons->setFocus();
}

void RegistrationWidget::onNextClicked()
{
    const auto& first = m_bulkEdit->get<QLineEdit*>(First)->text();
    const auto& last = m_bulkEdit->get<QLineEdit*>(Last)->text();
    const auto& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    const auto& cemail = m_bulkEdit->get<QLineEdit*>(ConfirmEmail)->text();
    const auto& password = m_bulkEdit->get<QLineEdit*>(Password)->text();
    const auto& cpassword = m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->text();
    const auto& company = m_bulkEdit->get<QLineEdit*>(Company)->text();
    const auto& title = m_bulkEdit->get<QLineEdit*>(Title)->text();
    const auto& phone = m_bulkEdit->get<QLineEdit*>(Phone)->text();
    const auto& country = m_bulkEdit->get<QComboBox*>(Country)->currentText();

    if (!m_termsSwitch->isChecked()) {
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
//        m_bulkEdit->get<QLineEdit*>(First)->text(),
//        m_bulkEdit->get<QLineEdit*>(Last)->text(),
//        m_bulkEdit->get<QLineEdit*>(Email)->text(),
//        m_bulkEdit->get<QLineEdit*>(Password)->text(),
//        static_cast<QComboBox*>(m_bulkEdit->get(Country)->currentText() != "Please select..." ?
//        static_cast<QComboBox*>(m_bulkEdit->get(Country)->currentText() : "",
//        m_bulkEdit->get<QLineEdit*>(Company)->text(),
//        m_bulkEdit->get<QLineEdit*>(Title)->text(),
//        m_bulkEdit->get<QLineEdit*>(Phone)->text()
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
