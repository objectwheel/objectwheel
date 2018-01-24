#include <loginwidget.h>
#include <fit.h>
#include <switch.h>
#include <bulkedit.h>
#include <buttonslice.h>
#include <flatbutton.h>
#include <waitingspinnerwidget.h>
#include <global.h>

#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>

#define AUTOLOGIN_HEIGHT (fit::fx(35))
#define AUTOLOGIN_WIDTH  (fit::fx(300))
#define SIZE_LOGO        (QSize(fit::fx(160), fit::fx(80)))
#define PATH_LOGO        (":/resources/images/logo.png")
#define PATH_RICON       (":/resources/images/new.png")
#define PATH_LICON       (":/resources/images/load.png")
#define PATH_HICON       (":/resources/images/info.png")
#define pS               (QApplication::primaryScreen())

enum Fields { Email, Password };
enum Buttons { Login, Register };

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent)
{
    _layout = new QGridLayout(this);
    _logoLabel = new QLabel;
    _loginLabel = new QLabel;
    _bulkEdit = new BulkEdit;
    _autologinWidget = new QWidget;
    _autologinLayout = new QHBoxLayout(_autologinWidget);
    _autologinSwitch = new Switch;
    _autologinLabel = new QLabel;
    _buttons = new ButtonSlice;
    _helpButton = new FlatButton;
    _loadingIndicator = new WaitingSpinnerWidget(this, false, false);
    _legalLabel = new QLabel;

    _layout->setSpacing(fit::fx(12));
    _layout->setRowStretch(0, 1);
    _layout->setRowStretch(1, 1);
    _layout->setRowStretch(8, 1);
    _layout->setRowStretch(10, 1);
    _layout->setColumnStretch(0, 1);
    _layout->setColumnStretch(2, 1);

    _layout->addWidget(_logoLabel, 2, 1);
    _layout->addWidget(_loginLabel, 3, 1);
    _layout->addWidget(_bulkEdit, 4, 1);
    _layout->addWidget(_autologinWidget, 5, 1);
    _layout->addWidget(_buttons, 6, 1);
    _layout->addWidget(_helpButton, 7, 1);
    _layout->addWidget(_loadingIndicator, 9, 1);
    _layout->addWidget(_legalLabel, 11, 1);
    _layout->setAlignment(_logoLabel, Qt::AlignCenter);
    _layout->setAlignment(_loginLabel, Qt::AlignCenter);
    _layout->setAlignment(_bulkEdit, Qt::AlignCenter);
    _layout->setAlignment(_autologinWidget, Qt::AlignCenter);
    _layout->setAlignment(_buttons, Qt::AlignCenter);
    _layout->setAlignment(_helpButton, Qt::AlignCenter);
    _layout->setAlignment(_loadingIndicator, Qt::AlignCenter);
    _layout->setAlignment(_legalLabel, Qt::AlignCenter);

    _autologinLayout->setSpacing(fit::fx(5));
    _autologinLayout->setContentsMargins(fit::fx(2.5), 0, 0, 0);
    _autologinLayout->addWidget(_autologinSwitch);
    _autologinLayout->addWidget(_autologinLabel);
    _autologinLayout->setAlignment(_autologinLabel, Qt::AlignVCenter);
    _autologinLayout->setAlignment(_autologinSwitch, Qt::AlignVCenter);
    _autologinLayout->addStretch();

    QPixmap p(PATH_LOGO);
    p.setDevicePixelRatio(pS->devicePixelRatio());

    _logoLabel->setFixedSize(SIZE_LOGO);
    _logoLabel->setPixmap(
        p.scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));

    _loginLabel->setFont(f);
    _loginLabel->setText(tr("Log In"));
    _loginLabel->setStyleSheet("color: #2E3A41");

    _bulkEdit->add(Email, tr("Email"));
    _bulkEdit->add(Password, tr("Password"));
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setEchoMode(QLineEdit::Password);
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->setText(tr("alpha@objectwheel.com"));
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setText(tr("password"));
    static_cast<QLineEdit*>(_bulkEdit->get(Email))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    static_cast<QLineEdit*>(_bulkEdit->get(Password))->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    _autologinWidget->setObjectName("autologinWidget");
    _autologinWidget->setFixedSize(AUTOLOGIN_WIDTH, AUTOLOGIN_HEIGHT);
    _autologinWidget->setStyleSheet(
        tr(
            "#autologinWidget {"
            "    border-radius: %1;"
            "    background: #12000000;"
            "    border: 1px solid #18000000;"
            "}"
        )
        .arg(int(AUTOLOGIN_HEIGHT / 2.0))
    );

    _autologinLabel->setText(tr("Automatic login"));
    _autologinLabel->setStyleSheet("color: #2E3A41");

    _autologinSwitch->setChecked(true);

    _buttons->add(Register, "#5BC5F8", "#2592F9");
    _buttons->add(Login, "#8BBB56", "#6EA045");
    _buttons->get(Register)->setText(tr("Sign Up"));
    _buttons->get(Login)->setText(tr("Log In"));
    _buttons->get(Register)->setIcon(QIcon(PATH_RICON));
    _buttons->get(Login)->setIcon(QIcon(PATH_LICON));
    _buttons->get(Register)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Login)->setCursor(Qt::PointingHandCursor);
    connect(_buttons->get(Register), SIGNAL(clicked(bool)), SIGNAL(signup()));

    _helpButton->setIconButton(true);
    _helpButton->setIcon(QIcon(PATH_HICON));
    _helpButton->setFixedSize(fit::fx(18),fit::fx(18));

    _loadingIndicator->setStyleSheet("background: transparent;");
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
    _legalLabel->setAlignment(Qt::AlignHCenter);
}