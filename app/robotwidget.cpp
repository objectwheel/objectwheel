#include <robotwidget.h>
#include <fit.h>
#include <buttonslice.h>
#include <waitingspinnerwidget.h>
#include <dpr.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWebEngineView>
#include <QWebChannel>
#include <QWebEngineSettings>
#include <QMessageBox>

#define BUTTONS_WIDTH    (fit::fx(350))
#define SIZE_ICON        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_ICON        (":/resources/images/robot.png")
#define PATH_NICON       (":/resources/images/load.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define PATH_RECAPTCHA   (tr(APP_HTTPSSERVER) + "/recaptcha.html")

enum Buttons { Next, Back };

RobotWidget::RobotWidget(QWidget* parent) : QWidget(parent)
{
    _layout = new QVBoxLayout(this);
    _webChannel = new QWebChannel(this);
    _recaptchaView = new QWebEngineView;
    _recaptchaWidget = new QWidget(_recaptchaView);
    _recaptchaLayout = new QVBoxLayout(_recaptchaWidget);
    _iconLabel = new QLabel;
    _robotLabel = new QLabel;
    _space = new QWidget;
    _loadingIndicator = new WaitingSpinnerWidget(this, false);
    _buttons = new ButtonSlice(_recaptchaView);

    _layout->setSpacing(0);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_recaptchaView);

    _space->setFixedSize(BUTTONS_WIDTH, fit::fx(200));
    _space->installEventFilter(this);

    _recaptchaLayout->setSpacing(fit::fx(12));
    _recaptchaLayout->addStretch();
    _recaptchaLayout->addWidget(_iconLabel, 0, Qt::AlignCenter);
    _recaptchaLayout->addWidget(_robotLabel, 0, Qt::AlignCenter);
    _recaptchaLayout->addStretch();
    _recaptchaLayout->addWidget(_loadingIndicator, 0, Qt::AlignCenter);
    _recaptchaLayout->addWidget(_space, 0 , Qt::AlignCenter);
    _recaptchaLayout->addStretch();

    connect(_recaptchaView, SIGNAL(loadStarted()),
      _loadingIndicator, SLOT(start()));
    connect(_recaptchaView, SIGNAL(loadFinished(bool)),
      _loadingIndicator, SLOT(stop()));

    _webChannel->registerObject("cpp", this);
    _recaptchaView->page()->setWebChannel(_webChannel);
    _recaptchaView->page()->setBackgroundColor(Qt::transparent);

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(DPR);

    _iconLabel->setFixedSize(SIZE_ICON);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * DPR,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));

    _robotLabel->setFont(f);
    _robotLabel->setText(tr("I'm not Robot"));
    _robotLabel->setStyleSheet("color: black");

    _buttons->add(Back, "#5BC5F8", "#2592F9");
    _buttons->add(Next, "#8BBB56", "#6EA045");
    _buttons->get(Next)->setText(tr("Next"));
    _buttons->get(Back)->setText(tr("Back"));
    _buttons->get(Next)->setIcon(QIcon(PATH_NICON));
    _buttons->get(Back)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Next)->setIconSize(fit::fx(QSizeF(16, 16)).toSize());
    _buttons->get(Back)->setIconSize(fit::fx(QSizeF(16, 16)).toSize());
    _buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Back)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 2.0;
    _buttons->triggerSettings();

    connect(_buttons->get(Next), SIGNAL(clicked(bool)),
      SLOT(onNextClicked()));
    connect(_buttons->get(Back), SIGNAL(clicked(bool)),
      SIGNAL(back()));

    _loadingIndicator->setStyleSheet("background: transparent;");
    _loadingIndicator->setColor(Qt::black);
    _loadingIndicator->setRoundness(50);
    _loadingIndicator->setMinimumTrailOpacity(5);
    _loadingIndicator->setTrailFadePercentage(100);
    _loadingIndicator->setRevolutionsPerSecond(2);
    _loadingIndicator->setNumberOfLines(12);
    _loadingIndicator->setLineLength(5);
    _loadingIndicator->setInnerRadius(4);
    _loadingIndicator->setLineWidth(2);
    _loadingIndicator->start();

    _recaptchaWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
    _buttons->raise();
}

const QString& RobotWidget::response() const
{
    return _response;
}

void RobotWidget::load()
{
    if (_recaptchaView->page()->url().isEmpty())
        reload();
}

void RobotWidget::reload()
{
    _recaptchaView->load(QUrl(PATH_RECAPTCHA));
}

void RobotWidget::reset()
{
    _recaptchaView->page()->runJavaScript("grecaptcha.reset();");
    _response.clear();
    _buttons->raise();
}

void RobotWidget::captchaExpired()
{
    _response.clear();
    _buttons->raise();
}

void RobotWidget::updateResponse(const QString& response)
{
    _response = response;
}

void RobotWidget::onNextClicked()
{
    if (_response.isEmpty())
        QMessageBox::warning(
            this,
            tr("Oops"),
            tr("Please pass the test first.")
        );
    else
        emit done(_response);
}

void RobotWidget::resizeEvent(QResizeEvent* event)
{
    _recaptchaWidget->setGeometry(rect());
    QWidget::resizeEvent(event);
}

bool RobotWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == _space && event->type() == QEvent::Move)
        _buttons->move(_space->geometry().bottomLeft());
    return QWidget::eventFilter(watched, event);
}
