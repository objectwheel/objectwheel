#include <loginwidget.h>
#include <fit.h>
#include <QPainter>
#include <QApplication>
#include <QScreen>

#define SIZE_BLOCK (fit::fx(342))
#define PATH_LOGO  (":/resources/images/logo.png")
#define SIZE_LOGO  (QSize(fit::fx(160), fit::fx(80)))
#define SIZE_INPUT (QSize(fit::fx(300), fit::fx(70)))
#define pS         (QApplication::primaryScreen())

static QPixmap* logoPixmap;

LoginWidget::LoginWidget(QWidget *parent) : QWidget(parent)
{
    static auto px(
        QPixmap(PATH_LOGO).scaled(
            SIZE_LOGO * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );
    logoPixmap = &px;
}

void LoginWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen("#2E3A41");

    QFont f;
    int spacing = fit::fx(12);
    int x = width() / 2.0 - SIZE_LOGO.width()/2.0;
    int y = height() / 2.0 - SIZE_BLOCK/2.0;

    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));
    painter.setFont(f);

    QRectF rect(
        x, y,
        SIZE_LOGO.width(),
        SIZE_LOGO.height()
    );

    painter.drawPixmap(
        rect, *logoPixmap,
        QRectF(
            QPointF(),
            rect.size() * pS->devicePixelRatio()
        )
    );

    x = width() / 2.0 - SIZE_BLOCK/2.0;
    y += (spacing + SIZE_LOGO.height());

    painter.drawText(
        QRectF(
            QPointF(x, y),
            QSizeF(SIZE_BLOCK, fit::fx(22))
        ),
        tr("Login"),
        QTextOption(Qt::AlignCenter)
    );

    x = width() / 2.0 - SIZE_INPUT.width()/2.0;
    y += (spacing + fit::fx(22));

    rect = QRectF(
        x, y,
        SIZE_INPUT.width(),
        SIZE_INPUT.height()
    );

    painter.setFont(QFont());
    painter.setPen("#18000000");
    painter.setBrush(QBrush("#12000000"));
    painter.drawRoundedRect(rect, fit::fx(8), fit::fx(8));
    painter.drawLine(x, rect.center().y(), x + SIZE_INPUT.width(), rect.center().y());

    painter.setPen("#40000000");
    painter.drawText(
        rect.adjusted(fit::fx(10), 0, 0, -SIZE_INPUT.height() / 2.0),
        tr("Email"),
        QTextOption(Qt::AlignVCenter)
    );
    painter.drawText(
        rect.adjusted(fit::fx(10), SIZE_INPUT.height() / 2.0, 0, 0),
        tr("Password"),
        QTextOption(Qt::AlignVCenter)
    );

    y += (spacing + SIZE_INPUT.height());

    rect = QRectF(
        x, y,
        SIZE_INPUT.width(),
        SIZE_INPUT.height() / 2.0
    );

    painter.setPen("#18000000");
    painter.drawRoundedRect(rect, rect.height() / 2.0, rect.height() / 2.0);

    painter.setPen("#2E3A41");
    painter.drawText(
        rect.adjusted(fit::fx(55), 0, 0, 0),
        tr("Automatic login"),
        QTextOption(Qt::AlignVCenter)
    );
}
