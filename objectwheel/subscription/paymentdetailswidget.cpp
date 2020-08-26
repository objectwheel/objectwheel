#include <paymentdetailswidget.h>
#include <bulkedit.h>
#include <paintutils.h>
#include <buttonslice.h>

#include <QLabel>
#include <QPushButton>
#include <QBoxLayout>
#include <QRegularExpressionValidator>

enum Fields { CardNumber, CardCvv, CardExpDate};
enum Buttons { Next, Back };

PaymentDetailsWidget::PaymentDetailsWidget(QWidget* parent) : QWidget(parent)
  , m_selectedPlanLabel(new QLabel(this))
  , m_bulkEdit(new BulkEdit(this))
{
    auto iconLabel = new QLabel(this);
    iconLabel->setFixedSize(QSize(60, 60));
    iconLabel->setPixmap(PaintUtils::pixmap(QStringLiteral(":/images/subscription/payment-details.svg"), QSize(60, 60), this));

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(16);

    auto titleLabel = new QLabel(this);
    titleLabel->setFont(f);
    titleLabel->setText(tr("Payment Details"));

    m_selectedPlanLabel->setAlignment(Qt::AlignHCenter);
    m_selectedPlanLabel->setStyleSheet(QStringLiteral("color: #77000000"));

    m_bulkEdit->add(CardNumber, tr("Card Number"));
    m_bulkEdit->add(CardExpDate, tr("Card Expiration Date"));
    m_bulkEdit->add(CardCvv, tr("Card Security Code"));
    m_bulkEdit->setFixedWidth(300);

    m_bulkEdit->get<QLineEdit*>(CardNumber)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CardExpDate)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_bulkEdit->get<QLineEdit*>(CardCvv)->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

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

    auto buttons = new ButtonSlice(this);
    buttons->add(Back, QLatin1String("#5BC5F8"), QLatin1String("#2592F9"));
    buttons->add(Next, QLatin1String("#86CC63"), QLatin1String("#75B257"));
    buttons->get(Back)->setText(tr("Back"));
    buttons->get(Next)->setText(tr("Next"));
    buttons->get(Back)->setIcon(QIcon(QStringLiteral(":/images/welcome/unload.png")));
    buttons->get(Next)->setIcon(QIcon(QStringLiteral(":/images/welcome/load.png")));
    buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    buttons->settings().cellWidth = m_bulkEdit->sizeHint().width() / 2.0;
    buttons->triggerSettings();

    auto layout = new QVBoxLayout(this);
    layout->setSpacing(6);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    layout->addWidget(iconLabel, 0, Qt::AlignHCenter);
    layout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_selectedPlanLabel, 0, Qt::AlignHCenter);
    layout->addWidget(m_bulkEdit, 0, Qt::AlignHCenter);
    layout->addWidget(buttons, 0, Qt::AlignHCenter);
    layout->addStretch();

    connect(m_bulkEdit->get<QLineEdit*>(CardNumber), &QLineEdit::editingFinished, this, [this] {
        QString text = m_bulkEdit->get<QLineEdit*>(CardNumber)->text();
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
        m_bulkEdit->get<QLineEdit*>(CardNumber)->setText(text);
    });
    connect(m_bulkEdit->get<QLineEdit*>(CardNumber), &QLineEdit::textEdited, this, [this] {
        if (m_bulkEdit->get<QLineEdit*>(CardNumber)->cursorPosition() !=
                m_bulkEdit->get<QLineEdit*>(CardNumber)->text().size())
            return;
        QString text = m_bulkEdit->get<QLineEdit*>(CardNumber)->text();
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
        m_bulkEdit->get<QLineEdit*>(CardNumber)->setText(text);
    });
    connect(m_bulkEdit->get<QLineEdit*>(CardExpDate), &QLineEdit::textChanged, this, [this] (QString text) {
        text = text.simplified();
        text.remove(QLatin1Char('/'));
        if (m_bulkEdit->get<QLineEdit*>(CardExpDate)->inputMask().isEmpty()) {
            if (!text.isEmpty()) {
                m_bulkEdit->get<QLineEdit*>(CardExpDate)->setInputMask(QStringLiteral("99/99;_"));
                m_bulkEdit->get<QLineEdit*>(CardExpDate)->cursorForward(false);
            }
        } else {
            if (text.isEmpty())
                m_bulkEdit->get<QLineEdit*>(CardExpDate)->setInputMask(QString());
        }
    });
    connect(buttons->get(Back), &QPushButton::clicked,
            this, &PaymentDetailsWidget::back);
    connect(m_bulkEdit, &BulkEdit::returnPressed,
            this, &PaymentDetailsWidget::next);
    //    connect(m_buttons->get(CompleteSignup), &QPushButton::clicked,
    //            this, &SignupVerificationWidget::onCompleteSignupClicked);
    //    connect(m_buttons->get(ResendSignupCode), &QPushButton::clicked,
    //            this, &SignupVerificationWidget::onResendSignupCodeClicked);
    //    connect(ApiManager::instance(), &ApiManager::completeSignupSuccessful,
    //            this, &SignupVerificationWidget::onCompleteSignupSuccessful);
    //    connect(ApiManager::instance(), &ApiManager::completeSignupFailure,
    //            this, &SignupVerificationWidget::onCompleteSignupFailure);
    //    connect(ApiManager::instance(), &ApiManager::resendSignupCodeSuccessful,
    //            this, &SignupVerificationWidget::onResendSignupCodeSuccessful);
    //    connect(ApiManager::instance(), &ApiManager::resendSignupCodeFailure,
    //            this, &SignupVerificationWidget::onResendSignupCodeFailure);
    //    connect(m_countdown, &Countdown::finished,
    //            this, &SignupVerificationWidget::onCountdownFinished);
    //    connect(ServerManager::instance(), &ServerManager::disconnected,
    //            this, &SignupVerificationWidget::onServerDisconnected);
}

void PaymentDetailsWidget::refresh(const PlanInfo& planInfo, qint64 selectedPlan)
{
    int col = planInfo.columnForIdentifier(selectedPlan);
    qreal price = planInfo.price(col);
    qreal annualPrice = planInfo.annualPrice(col);
    QString priceText = tr("$%1/month").arg(price);
    if (annualPrice >= 0)
        priceText += tr(" ($%2 if paid annually)").arg(annualPrice);
    m_planInfo = planInfo;
    m_selectedPlan = selectedPlan;
    m_selectedPlanLabel->setText(tr("Please enter your payment card and<br>"
                                    "billing details below to continue purchasing<br>"
                                    "<span style=\"font-weight: 500\">Plan: </span>%1"
                                    "<span style=\"font-weight: 500\">, Price: </span>%2")
                                 .arg(planInfo.at(0, col)).arg(priceText));
}
