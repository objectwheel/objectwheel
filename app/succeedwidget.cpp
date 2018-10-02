#include <succeedwidget.h>
#include <buttonslice.h>
#include <utilityfunctions.h>

#include <QTimer>
#include <QMovie>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QPainter>

#define BUTTONS_WIDTH    (150)
#define SIZE_GIF        (QSize(100, 100))
#define PATH_GIF         (":/images/complete.gif")
#define PATH_OICON       (":/images/ok.png")

enum Buttons { Ok };

SucceedWidget::SucceedWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _movie = new QMovie(this);
    _iconLabel = new QLabel;
    _titleLabel = new QLabel;
    _descriptionLabel = new QLabel;
    _buttons = new ButtonSlice;

    _layout->setSpacing(12);
    _layout->addStretch();
    _layout->addWidget(_iconLabel);
    _layout->addWidget(_titleLabel);
    _layout->addWidget(_descriptionLabel);
    _layout->addWidget(_buttons);
    _layout->addStretch();

    _layout->setAlignment(_iconLabel, Qt::AlignCenter);
    _layout->setAlignment(_titleLabel, Qt::AlignCenter);
    _layout->setAlignment(_descriptionLabel, Qt::AlignCenter);
    _layout->setAlignment(_buttons, Qt::AlignCenter);

    _movie->setFileName(PATH_GIF);
    _movie->setBackgroundColor(Qt::transparent);
    _movie->setScaledSize(SIZE_GIF * devicePixelRatioF());
    connect(_movie, SIGNAL(frameChanged(int)), SLOT(update()));

    _iconLabel->setFixedSize(SIZE_GIF);
    _iconLabel->setStyleSheet("background: transparent;");

    UtilityFunctions::adjustFontPixelSize(_titleLabel, 3);
    _titleLabel->setStyleSheet("color: #65A35C");

    _descriptionLabel->setStyleSheet("color: #304050");
    _descriptionLabel->setAlignment(Qt::AlignHCenter);

    _buttons->add(Ok, "#86CC63", "#75B257");
    _buttons->get(Ok)->setText(tr("Ok"));
    _buttons->get(Ok)->setIcon(QIcon(PATH_OICON));
    _buttons->get(Ok)->setIconSize(QSize(16, 16));
    _buttons->get(Ok)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH;
    _buttons->triggerSettings();
    connect(_buttons->get(Ok), SIGNAL(clicked(bool)), SIGNAL(done()));
}

void SucceedWidget::start()
{
    _movie->jumpToFrame(0);
    update();
    QTimer::singleShot(20, _movie, &QMovie::start);
}

void SucceedWidget::update(const QString& title, const QString& description)
{
    _titleLabel->setText(title);
    _descriptionLabel->setText(description);
}

void SucceedWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    if (_movie->currentFrameNumber() > 48) {
        auto r = _iconLabel->geometry().adjusted(10, 10, -10, -10);
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(r, r.width() / 2.0, r.width() / 2.0);
    }

    painter.drawPixmap(
        _iconLabel->geometry(),
        _movie->currentPixmap(),
        QRectF(QPoint(), SIZE_GIF * devicePixelRatioF())
    );
}
