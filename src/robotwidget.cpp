#include <robotwidget.h>
#include <fit.h>
#include <buttonslice.h>
#include <internetaccess.h>

#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWebEngineView>
#include <QWebChannel>
#include <QWebEngineSettings>

#define BUTTONS_WIDTH    (fit::fx(350))
#define SIZE_ICON        (QSize(fit::fx(80), fit::fx(80)))
#define PATH_ICON        (":/resources/images/robot.png")
#define PATH_NICON       (":/resources/images/ok.png")
#define PATH_CICON       (":/resources/images/unload.png")
#define PATH_RECAPTCHA   ("qrc://resources/other/recaptcha.html")
#define pS               (QApplication::primaryScreen())

enum Buttons { Next, Cancel };
#include <QWebEngineHistory>
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
    _buttons = new ButtonSlice(_recaptchaView);

    _layout->setSpacing(0);
    _layout->setContentsMargins(0, 0, 0, 0);
    _layout->addWidget(_recaptchaView);

    _space->setFixedSize(BUTTONS_WIDTH, fit::fx(200));

    _recaptchaLayout->setSpacing(fit::fx(12));
    _recaptchaLayout->addStretch();
    _recaptchaLayout->addWidget(_iconLabel, 0 , Qt::AlignCenter);
    _recaptchaLayout->addWidget(_robotLabel, 0 , Qt::AlignCenter);
    _recaptchaLayout->addWidget(_space, 0 , Qt::AlignCenter);
    _recaptchaLayout->addStretch();
    _recaptchaLayout->addStretch();

    _webChannel->registerObject("cpp", this);
    _recaptchaView->page()->setWebChannel(_webChannel);
    _recaptchaView->page()->setBackgroundColor(Qt::transparent);

    QPixmap p(PATH_ICON);
    p.setDevicePixelRatio(pS->devicePixelRatio());

    _iconLabel->setFixedSize(SIZE_ICON);
    _iconLabel->setPixmap(
        p.scaled(
            SIZE_ICON * pS->devicePixelRatio(),
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        )
    );

    QFont f;
    f.setWeight(QFont::Light);
    f.setPixelSize(fit::fx(18));

    _robotLabel->setFont(f);
    _robotLabel->setText(tr("I'm not robot"));
    _robotLabel->setStyleSheet("color: #2E3A41");

    _buttons->add(Cancel, "#cf5751", "#B34B46");
    _buttons->add(Next, "#6ab35f", "#599750");
    _buttons->get(Next)->setText(tr("Next"));
    _buttons->get(Cancel)->setText(tr("Cancel"));
    _buttons->get(Next)->setIcon(QIcon(PATH_NICON));
    _buttons->get(Cancel)->setIcon(QIcon(PATH_CICON));
    _buttons->get(Next)->setCursor(Qt::PointingHandCursor);
    _buttons->get(Cancel)->setCursor(Qt::PointingHandCursor);
    _buttons->settings().cellWidth = BUTTONS_WIDTH / 2.0;
    _buttons->triggerSettings();
    _buttons->get(Next)->setEnabled(false);

    connect(_buttons->get(Next), SIGNAL(clicked(bool)), SLOT(onNextClicked()));
    connect(_buttons->get(Cancel), SIGNAL(clicked(bool)), SIGNAL(cancel()));

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
        _recaptchaView->load(QUrl("http://localhost"));
}

void RobotWidget::reset()
{
    _recaptchaView->page()->runJavaScript("grecaptcha.reset();");
    _buttons->get(Next)->setEnabled(false);
    _buttons->raise();
}

void RobotWidget::captchaExpired()
{
    _buttons->get(Next)->setEnabled(false);
    _buttons->raise();
}

void RobotWidget::updateResponse(const QString& response)
{
    if (!response.isEmpty()) {
        _response = response;
        _buttons->get(Next)->setEnabled(true);
    }
}

void RobotWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    _recaptchaWidget->setGeometry(rect());
    _buttons->move(_space->geometry().bottomLeft());
}

void RobotWidget::onNextClicked()
{
    emit done(_response);
}