#include <loginwidget.h>
#include <fit.h>
#include <switch.h>
#include <bulkedit.h>

#include <QPainter>
#include <QApplication>
#include <QScreen>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

#define SIZE_SPACING (fit::fx(12))
#define SIZE_BLOCK   (fit::fx(342))
#define PATH_LOGO    (":/resources/images/logo.png")
#define SIZE_LOGO    (QSize(fit::fx(160), fit::fx(80)))
#define HEIGHT       (fit::fx(35))
#define WIDTH        (fit::fx(300))
#define pS           (QApplication::primaryScreen())

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

    _layout->setSpacing(SIZE_SPACING);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->setRowStretch(0, 1);
    _layout->setRowStretch(5, 1);
    _layout->setColumnStretch(0, 1);
    _layout->setColumnStretch(2, 1);

    _logoLabel->setScaledContents(true);
    _logoLabel->setFixedSize(SIZE_LOGO);
    _logoLabel->setPixmap(
        QPixmap(PATH_LOGO).scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    _layout->addWidget(_logoLabel, 1, 1);
    _layout->addWidget(_loginLabel, 2, 1);
    _layout->addWidget(_bulkEdit, 3, 1);
    _layout->addWidget(_autologinWidget, 4, 1);
    _layout->setAlignment(_logoLabel, Qt::AlignCenter);
    _layout->setAlignment(_loginLabel, Qt::AlignCenter);
    _layout->setAlignment(_bulkEdit, Qt::AlignCenter);
    _layout->setAlignment(_autologinWidget, Qt::AlignCenter);

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));
    _loginLabel->setFont(f);
    _loginLabel->setText(tr("Login"));
    _loginLabel->setStyleSheet("color: #2E3A41");

    _bulkEdit->add(0, tr("Email"));
    _bulkEdit->add(1, tr("Password"));
    _bulkEdit->setFixedWidth(WIDTH);

    _autologinWidget->setObjectName("autologinWidget");
    _autologinWidget->setStyleSheet(
        tr(
            "#autologinWidget {"
            "    border-radius: %1;"
            "    background: #12000000;"
            "    border: 1px solid #18000000;"
            "}"
        )
        .arg(int(HEIGHT / 2.0))
    );
    _autologinWidget->setFixedSize(WIDTH, HEIGHT);
    _autologinLabel->setStyleSheet("color: #2E3A41");
    _autologinLabel->setText(tr("Automatic login"));

    _autologinLayout->setSpacing(SIZE_SPACING / 2.0);
    _autologinLayout->setContentsMargins(fit::fx(2.5), 0, 0, 0);
    _autologinLayout->addWidget(_autologinSwitch);
    _autologinLayout->addWidget(_autologinLabel);
    _autologinLayout->setAlignment(_autologinLabel, Qt::AlignVCenter);
    _autologinLayout->setAlignment(_autologinSwitch, Qt::AlignVCenter);
    _autologinLayout->addStretch();
}

//void LoginWidget::paintEvent(QPaintEvent*)
//{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);
//    painter.setPen("#2E3A41");

//    QFont f;
//    int spacing = fit::fx(12);
//    int x = width() / 2.0 - SIZE_LOGO.width()/2.0;
//    int y = height() / 2.0 - SIZE_BLOCK/2.0;

//    f.setWeight(QFont::Light);
//    f.setPixelSize(fit::fx(18));
//    painter.setFont(f);

//    QRectF rect(
//        x, y,
//        SIZE_LOGO.width(),
//        SIZE_LOGO.height()
//    );

//    painter.drawPixmap(
//        rect, *logoPixmap,
//        QRectF(
//            QPointF(),
//            rect.size() * pS->devicePixelRatio()
//        )
//    );

//    x = width() / 2.0 - SIZE_BLOCK/2.0;
//    y += (spacing + SIZE_LOGO.height());

//    painter.drawText(
//        QRectF(
//            QPointF(x, y),
//            QSizeF(SIZE_BLOCK, fit::fx(22))
//        ),
//        tr("Login"),
//        QTextOption(Qt::AlignCenter)
//    );

//    x = width() / 2.0 - SIZE_INPUT.width()/2.0;
//    y += (spacing + fit::fx(22));

//    rect = QRectF(
//        x, y,
//        SIZE_INPUT.width(),
//        SIZE_INPUT.height()
//    );

//    painter.setFont(QFont());
//    painter.setPen("#18000000");
//    painter.setBrush(QBrush("#12000000"));
//    painter.drawRoundedRect(rect, fit::fx(8), fit::fx(8));
//    painter.drawLine(x, rect.center().y(), x + SIZE_INPUT.width(), rect.center().y());

//    painter.setPen("#40000000");
//    painter.drawText(
//        rect.adjusted(fit::fx(10), 0, 0, -SIZE_INPUT.height() / 2.0),
//        tr("Email"),
//        QTextOption(Qt::AlignVCenter)
//    );
//    painter.drawText(
//        rect.adjusted(fit::fx(10), SIZE_INPUT.height() / 2.0, 0, 0),
//        tr("Password"),
//        QTextOption(Qt::AlignVCenter)
//    );

//    y += (spacing + SIZE_INPUT.height());

//    rect = QRectF(
//        x, y,
//        SIZE_INPUT.width(),
//        SIZE_INPUT.height() / 2.0
//    );

//    painter.setPen("#18000000");
//    painter.drawRoundedRect(rect, rect.height() / 2.0, rect.height() / 2.0);

//    painter.setPen("#2E3A41");
//    painter.drawText(
//        rect.adjusted(fit::fx(55), 0, 0, 0),
//        tr("Automatic login"),
//        QTextOption(Qt::AlignVCenter)
//    );
//}
