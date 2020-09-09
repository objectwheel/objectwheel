#include <paymentdetailswidget.h>
#include <paintutils.h>
#include <utilityfunctions.h>
#include <usermanager.h>
#include <bulkedit.h>
#include <buttonslice.h>

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QRegularExpressionValidator>
#include <QComboBox>
#include <QCborMap>

enum Fields { CardNumber, CardExpDate, CardCvv, FullName, Email, Phone, Country, State, City, Address, PostalCode };
enum Buttons { Next, Back };

PaymentDetailsWidget::PaymentDetailsWidget(QWidget* parent) : QWidget(parent)
  , m_selectedPlan(0)
  , m_bulkEdit(new BulkEdit(this))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/subscription/payment-details.svg"),
                                            QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("Payment Details"));

    auto descriptionLabel = new QLabel(this);
    descriptionLabel->setAlignment(Qt::AlignHCenter);
    descriptionLabel->setStyleSheet(QStringLiteral("color: #77000000"));
    descriptionLabel->setText(tr("Please enter your payment details below to continue"));

    m_bulkEdit->setFixedWidth(366);
    m_bulkEdit->add(CardNumber, tr("Card Number *"));
    m_bulkEdit->add(CardExpDate, tr("Card Expiration Date *"));
    m_bulkEdit->add(CardCvv, tr("Card Security Code *"));
    m_bulkEdit->add(FullName, tr("Full Name"));
    m_bulkEdit->add(Email, tr("Email Address"));
    m_bulkEdit->add(Phone, tr("Phone"));
    m_bulkEdit->add(Country, tr("Country"), new QComboBox(this));
    m_bulkEdit->add(State, tr("State"));
    m_bulkEdit->add(City, tr("City"));
    m_bulkEdit->add(Address, tr("Address"));
    m_bulkEdit->add(PostalCode, tr("Postal Code"));

    m_bulkEdit->get<QLineEdit*>(CardNumber)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CardExpDate)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CardCvv)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(FullName)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Email)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Phone)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(State)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(City)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(Address)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(PostalCode)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_bulkEdit->get<QLineEdit*>(CardNumber)->setPlaceholderText(tr("dddd dddd dddd dddd"));
    m_bulkEdit->get<QLineEdit*>(CardExpDate)->setPlaceholderText(tr("mm/yy"));
    m_bulkEdit->get<QLineEdit*>(CardCvv)->setPlaceholderText(tr("ddd"));

    m_bulkEdit->get<QLineEdit*>(CardNumber)->setInputMethodHints(
                m_bulkEdit->get<QLineEdit*>(CardNumber)->inputMethodHints() | Qt::ImhSensitiveData);
    m_bulkEdit->get<QLineEdit*>(CardExpDate)->setInputMethodHints(
                m_bulkEdit->get<QLineEdit*>(CardExpDate)->inputMethodHints() | Qt::ImhSensitiveData);
    m_bulkEdit->get<QLineEdit*>(CardCvv)->setInputMethodHints(
                m_bulkEdit->get<QLineEdit*>(CardCvv)->inputMethodHints() | Qt::ImhSensitiveData);

    m_bulkEdit->get<QLineEdit*>(CardNumber)->setValidator(
                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("[\\d ]{1,24}")), this));
    m_bulkEdit->get<QLineEdit*>(CardCvv)->setValidator(
                new QRegularExpressionValidator(QRegularExpression(QStringLiteral("\\d{1,6}")), this));

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

    auto buttons = new ButtonSlice(this);
    buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    buttons->add(Next, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    buttons->get(Back)->setText(tr("Back"));
    buttons->get(Next)->setText(tr("Next"));
    buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    buttons->get(Next)->setIcon(QIcon(QStringLiteral(":/images/welcome/load.png")));
    buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    buttons->settings().cellWidth = m_bulkEdit->width() / 2.0;
    buttons->triggerSettings();

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(descriptionLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(buttons, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(m_bulkEdit->get<QLineEdit*>(CardNumber), &QLineEdit::editingFinished,
            this, &PaymentDetailsWidget::onCardNumberEditTextEditingFinished);
    connect(m_bulkEdit->get<QLineEdit*>(CardNumber), &QLineEdit::textEdited,
            this, &PaymentDetailsWidget::onCardNumberEditTextEdited);
    connect(m_bulkEdit->get<QLineEdit*>(CardExpDate), &QLineEdit::textChanged,
            this, &PaymentDetailsWidget::onCardExpDateEditTextChanged);
    connect(buttons->get(Back), &QPushButton::clicked,
            this, &PaymentDetailsWidget::back);
    connect(buttons->get(Next), &QPushButton::clicked,
            this, &PaymentDetailsWidget::onNextClicked);
    connect(m_bulkEdit, &BulkEdit::returnPressed,
            this, &PaymentDetailsWidget::onNextClicked);
}

void PaymentDetailsWidget::clear()
{
    m_planInfo = PlanInfo();
    m_selectedPlan = 0;
    m_bulkEdit->get<QLineEdit*>(CardNumber)->clear();
    m_bulkEdit->get<QLineEdit*>(CardExpDate)->clear();
    m_bulkEdit->get<QLineEdit*>(CardCvv)->clear();
    m_bulkEdit->get<QLineEdit*>(FullName)->clear();
    m_bulkEdit->get<QLineEdit*>(Email)->clear();
    m_bulkEdit->get<QLineEdit*>(Phone)->clear();
    m_bulkEdit->get<QComboBox*>(Country)->setCurrentIndex(0);
    m_bulkEdit->get<QLineEdit*>(State)->clear();
    m_bulkEdit->get<QLineEdit*>(City)->clear();
    m_bulkEdit->get<QLineEdit*>(Address)->clear();
    m_bulkEdit->get<QLineEdit*>(PostalCode)->clear();
}

void PaymentDetailsWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan)
{
    m_planInfo = planInfo;
    m_selectedPlan = selectedPlan;
    if (m_bulkEdit->get<QLineEdit*>(Email)->text().isEmpty())
        m_bulkEdit->get<QLineEdit*>(Email)->setText(UserManager::email());
}

void PaymentDetailsWidget::onCardNumberEditTextEditingFinished()
{
    auto cardNumberEdit = m_bulkEdit->get<QLineEdit*>(CardNumber);
    QString text = cardNumberEdit->text();
    text.remove(QRegularExpression(QStringLiteral("[^\\d]")));
    if (text.size() > 20)
        text.chop(text.size() - 20);
    QStringList parts;
    while(!text.isEmpty()) {
        parts.append(text.left(4));
        text.remove(0, 4);
    }
    if (!parts.isEmpty())
        text = parts.join(QLatin1Char(' '));
    cardNumberEdit->setText(text);
}

void PaymentDetailsWidget::onCardNumberEditTextEdited(QString text)
{
    auto cardNumberEdit = m_bulkEdit->get<QLineEdit*>(CardNumber);
    if (cardNumberEdit->cursorPosition() != cardNumberEdit->text().size())
        return;
    text.remove(QRegularExpression(QStringLiteral("[^\\d]")));
    if (text.size() > 20)
        text.chop(text.size() - 20);
    QStringList parts;
    while(!text.isEmpty()) {
        parts.append(text.left(4));
        text.remove(0, 4);
    }
    if (!parts.isEmpty())
        text = parts.join(QLatin1Char(' '));
    cardNumberEdit->setText(text);
}

void PaymentDetailsWidget::onCardExpDateEditTextChanged(QString text)
{
    auto cardExpDateEdit = m_bulkEdit->get<QLineEdit*>(CardExpDate);
    text = text.simplified();
    text.remove(QLatin1Char('/'));
    if (cardExpDateEdit->inputMask().isEmpty()) {
        if (!text.isEmpty()) {
            cardExpDateEdit->setInputMask(QStringLiteral("99/99;_"));
            cardExpDateEdit->cursorForward(false);
        }
    } else {
        if (text.isEmpty())
            cardExpDateEdit->setInputMask(QString());
    }
}

void PaymentDetailsWidget::onNextClicked()
{
    QString cardNumber = m_bulkEdit->get<QLineEdit*>(CardNumber)->text();
    QString cardExpDate = m_bulkEdit->get<QLineEdit*>(CardExpDate)->text();
    QString cardCvv = m_bulkEdit->get<QLineEdit*>(CardCvv)->text();
    const QString& fullName = m_bulkEdit->get<QLineEdit*>(FullName)->text();
    const QString& email = m_bulkEdit->get<QLineEdit*>(Email)->text();
    const QString& phone = m_bulkEdit->get<QLineEdit*>(Phone)->text();
    const QString& countryCode = UtilityFunctions::countryList().value(m_bulkEdit->get<QComboBox*>(Country)->currentText()).toString();
    const QString& state = m_bulkEdit->get<QLineEdit*>(State)->text();
    const QString& city = m_bulkEdit->get<QLineEdit*>(City)->text();
    const QString& address = m_bulkEdit->get<QLineEdit*>(Address)->text();
    const QString& postalCode = m_bulkEdit->get<QLineEdit*>(PostalCode)->text();

    cardNumber.remove(QRegularExpression("[^\\d]"));
    cardExpDate.remove(QRegularExpression("[^\\d\\/]"));
    cardCvv.remove(QRegularExpression("[^\\d]"));

    if (cardNumber.isEmpty() || cardExpDate.isEmpty() || cardCvv.isEmpty()) {
        UtilityFunctions::showMessage(this,
                                      tr("Required fields must be filled"),
                                      tr("Please fill in all the required fields."));
        return;
    }

    const QDate& cardExpirationDate = QDate::fromString(cardExpDate, "MM'/'yy").addYears(100);
    if (!cardExpirationDate.isValid() || cardExpirationDate < QDate::currentDate().addMonths(-1)) {
        UtilityFunctions::showMessage(this,
                                      tr("Card expiration date is invalid"),
                                      tr("Please enter a valid card expiration date."));
        return;
    }

    if (cardNumber.size() > 255
            || cardExpDate.size() > 255
            || cardCvv.size() > 255
            || fullName.size() > 255
            || email.size() > 255
            || phone.size() > 255
            || countryCode.size() > 255
            || state.size() > 255
            || city.size() > 255
            || address.size() > 255
            || postalCode.size() > 255) {
        UtilityFunctions::showMessage(this,
                                      tr("Entry too long"),
                                      tr("No field can be larger than 255 characters."));
        return;
    }

    if (!email.isEmpty() && !UtilityFunctions::isEmailFormatCorrect(email)) {
        UtilityFunctions::showMessage(this,
                                      tr("Corrupt email address"),
                                      tr("Your email address doesn't comply with "
                                         "the standard email address format."));
        return;
    }

    emit next(m_planInfo, m_selectedPlan, cardNumber, cardExpirationDate, cardCvv,
              fullName, email, phone, countryCode, state, city, address, postalCode);
}
