#include <signupwidget.h>
#include <appconstants.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <busyindicatorwidget.h>
#include <apimanager.h>
#include <utilityfunctions.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <servermanager.h>

#include <QBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCborMap>

enum Fields { First, Last, Email, ConfirmEmail, Password, ConfirmPassword, Country, Company, Title, Phone };
enum Buttons { Next, Back };

SignupWidget::SignupWidget(QWidget* parent) : QWidget(parent)
  , m_bulkEdit(new BulkEdit(this))
  , m_termsSwitch(new Switch(this))
  , m_buttons(new ButtonSlice(this))
  , m_busyIndicator(new BusyIndicatorWidget(this, false))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/welcome/register.svg"), QSize(60, 60), this));

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

    auto countryCombo = m_bulkEdit->get<QComboBox*>(Country);
    countryCombo->setFrame(false);
    countryCombo->setEditable(true);
    countryCombo->addItem(tr("Please select..."));
    foreach (const QCborValue& countryName, UtilityFunctions::countryList().keys())
        countryCombo->addItem(countryName.toString());
    countryCombo->lineEdit()->setFrame(false);
    countryCombo->lineEdit()->setReadOnly(true);
    countryCombo->lineEdit()->setAttribute(Qt::WA_TransparentForMouseEvents);
    countryCombo->lineEdit()->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    auto termsWidget = new QWidget(this);
    termsWidget->setFixedSize(m_bulkEdit->sizeHint().width(), 35);
    termsWidget->setObjectName(QStringLiteral("termsWidget"));
    termsWidget->setStyleSheet(QStringLiteral("#termsWidget {"
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
    termsLayout->setSpacing(4);
    termsLayout->setContentsMargins(2, 0, 0, 0);
    termsLayout->addWidget(m_termsSwitch, 0, Qt::AlignVCenter);
    termsLayout->addWidget(termsLabel, 0, Qt::AlignVCenter);
    termsLayout->addStretch();

    m_buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    m_buttons->add(Next, QLatin1String("#8BBB56"), QLatin1String("#6EA045"));
    m_buttons->get(Next)->setText(tr("Next"));
    m_buttons->get(Back)->setText(tr("Back"));
    m_buttons->get(Next)->setIcon(QIcon(QStringLiteral(":/images/welcome/load.png")));
    m_buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    m_buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    m_buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    m_buttons->settings().cellWidth = m_bulkEdit->sizeHint().width() / 2.0;
    m_buttons->triggerSettings();

    m_busyIndicator->setRoundness(50);
    m_busyIndicator->setMinimumTrailOpacity(5);
    m_busyIndicator->setTrailFadePercentage(100);
    m_busyIndicator->setRevolutionsPerSecond(2);
    m_busyIndicator->setNumberOfLines(12);
    m_busyIndicator->setLineLength(5);
    m_busyIndicator->setInnerRadius(4);
    m_busyIndicator->setLineWidth(2);

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(4);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(signupLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(termsWidget, 0, Qt::AlignHCenter);
    layout->addWidget(m_buttons, 0, Qt::AlignHCenter);
    layout->addStretch();
    layout->addWidget(m_busyIndicator, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(ServerManager::instance(), &ServerManager::disconnected,
            this, &SignupWidget::onServerDisconnected);
    connect(ApiManager::instance(), &ApiManager::signupSuccessful,
            this, &SignupWidget::onSignupSuccessful);
    connect(ApiManager::instance(), &ApiManager::signupFailure,
            this, &SignupWidget::onSignupFailure);
    connect(m_buttons->get(Next), &QPushButton::clicked,
            this, &SignupWidget::onNextClicked);
    connect(m_bulkEdit, &BulkEdit::returnPressed,
            this, &SignupWidget::onNextClicked);
    connect(m_buttons->get(Back), &QPushButton::clicked,
            this, &SignupWidget::back);
}

void SignupWidget::onNextClicked()
{
    const QString& first = m_bulkEdit->get<QLineEdit*>(First)->text();
    const QString& last = m_bulkEdit->get<QLineEdit*>(Last)->text();
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    const QString& cemail = m_bulkEdit->get<QLineEdit*>(ConfirmEmail)->text();
    const QString& password = m_bulkEdit->get<QLineEdit*>(Password)->text();
    const QString& cpassword = m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->text();
    const QString& country = m_bulkEdit->get<QComboBox*>(Country)->currentIndex() > 0
            ? m_bulkEdit->get<QComboBox*>(Country)->currentText() : QString();
    const QString& company = m_bulkEdit->get<QLineEdit*>(Company)->text();
    const QString& title = m_bulkEdit->get<QLineEdit*>(Title)->text();
    const QString& phone = m_bulkEdit->get<QLineEdit*>(Phone)->text();
    const QString& hash = UtilityFunctions::hashPassword(password);

    if (!m_termsSwitch->isChecked()) {
        UtilityFunctions::showMessage(this,
                                      tr("Please accept the terms and conditions"),
                                      tr("You have to agree with the terms and conditions "
                                         "in order to use %1 apps and services.").arg(AppConstants::NAME));
        return;
    }

    if (first.isEmpty()
            || last.isEmpty()
            || email.isEmpty()
            || cemail.isEmpty()
            || password.isEmpty()
            || cpassword.isEmpty()) {
        UtilityFunctions::showMessage(this,
                                      tr("Required fields must be filled"),
                                      tr("Please fill in all the required fields."),
                                      QMessageBox::Information);
        return;
    }

    if (first.size() > 255
            || last.size() > 255
            || email.size() > 255
            || cemail.size() > 255
            || password.size() > 255
            || cpassword.size() > 255
            || country.size() > 255
            || company.size() > 255
            || title.size() > 255
            || phone.size() > 255) {
        UtilityFunctions::showMessage(this,
                                      tr("Entry too long"),
                                      tr("No field can be larger than 255 characters."),
                                      QMessageBox::Information);
        return;
    }

    if (email != cemail) {
        UtilityFunctions::showMessage(this,
                                      tr("Incorrect email addresses"),
                                      tr("Email addresses you entered do not match."),
                                      QMessageBox::Information);
        return;
    }

    if (password != cpassword) {
        UtilityFunctions::showMessage(this,
                                      tr("Incorrect passwords"),
                                      tr("Passwords you entered do not match."),
                                      QMessageBox::Information);
        return;
    }

    if (!UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt email address"),
                                      tr("Your email address doesn't comply with "
                                         "the standard email address format."),
                                      QMessageBox::Information);
        return;
    }

    if (!UtilityFunctions::isPasswordFormatCorrect(password)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt password"),
                                      tr("Your password must comply with following standards:\n"
                                         "•  Length must be between 6 and 35 characters\n"
                                         "•  Only Latin-1 characters are allowed\n"
                                         "•  Whitespace characters are not allowed\n"
                                         "•  It can contain a-z, A-Z, 0-9\n"
                                         "•  It can also contain following special characters:\n"
                                         "   [ ] > < { } * ! @ - # $ % ^ & + = ~ . , :"),
                                      QMessageBox::Information);
        return;
    }

    if (ServerManager::isConnected()) {
        m_busyIndicator->start();
        ApiManager::signup(first, last, email, hash, country, company, title, phone);
    } else {
        UtilityFunctions::showMessage(this,
                                      tr("Unable to connect to the server"),
                                      tr("Please make sure you are connected to the internet."),
                                      QMessageBox::Information);
    }
}

void SignupWidget::onSignupSuccessful()
{
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    m_busyIndicator->stop();
    m_bulkEdit->get<QLineEdit*>(First)->clear();
    m_bulkEdit->get<QLineEdit*>(Last)->clear();
    m_bulkEdit->get<QLineEdit*>(Email)->clear();
    m_bulkEdit->get<QLineEdit*>(ConfirmEmail)->clear();
    m_bulkEdit->get<QLineEdit*>(Password)->clear();
    m_bulkEdit->get<QLineEdit*>(ConfirmPassword)->clear();
    m_bulkEdit->get<QLineEdit*>(Company)->clear();
    m_bulkEdit->get<QLineEdit*>(Title)->clear();
    m_bulkEdit->get<QLineEdit*>(Phone)->clear();
    m_bulkEdit->get<QComboBox*>(Country)->setCurrentIndex(0);
    m_termsSwitch->setChecked(false);
    emit done(email);
}

void SignupWidget::onSignupFailure()
{
    m_busyIndicator->stop();
    UtilityFunctions::showMessage(this,
                                  tr("Invalid information entered"),
                                  tr("The server rejected your request. Please review the information you entered "
                                     "and make sure you are not trying to sign up more than once."));
}

void SignupWidget::onServerDisconnected()
{
    if (m_busyIndicator->isSpinning()) {
        m_busyIndicator->stop();
        UtilityFunctions::showMessage(this,
                                      tr("Connection lost"),
                                      tr("We are unable to connect to the server."));
    }
}