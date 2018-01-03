#include <executivewidget.h>
#include <fit.h>
#include <QPainter>
#include <QSvgRenderer>
#include <QQmlEngine>
#include <QMouseEvent>
#include <QQuickItem>

ExecutiveWidget::ExecutiveWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(this)
    , _exitButton(this)
    , _x(0), _y(0)
    , _pressed(false)
{
    #if !defined(Q_OS_WIN)
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    #endif
    setAttribute(Qt::WA_QuitOnClose, false);

    _layout.setSpacing(0);
    _layout.setContentsMargins(0, 0, 0, 0);

    _containerWidget = createWindowContainer(&_window);
    _layout.addWidget(_containerWidget);
    _layout.setAlignment(_containerWidget, Qt::AlignCenter);

    _exitButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _exitButton.setColor("#C61717");
    _exitButton.setFixedSize(fit::fx(20),fit::fx(20));
    _exitButton.setRadius(fit::fx(9));
    _exitButton.setIconSize(QSize(fit::fx(17),fit::fx(17)));
    _exitButton.setIcon(QIcon(":/resources/images/down-arrow.png"));
    connect(&_exitButton, SIGNAL(clicked(bool)), SLOT(stop()));
    connect(qApp, SIGNAL(aboutToQuit()), SLOT(stop()));
}

Skin ExecutiveWidget::skin() const
{
    return _skin;
}

void ExecutiveWidget::setSkin(const Skin& skin)
{
    _skin = skin;
    if (_skin == PhonePortrait) {
        setFixedSize(SIZE_SKIN);
        _containerWidget->setFixedSize(SIZE_FORM);
        _exitButton.setGeometry(SIZE_SKIN.width() - fit::fx(55),
            fit::fx(26), fit::fx(20), fit::fx(20));
    } else {
        setFixedSize(SIZE_SKIN.transposed());
        _containerWidget->setFixedSize(SIZE_FORM.transposed());
        _exitButton.setGeometry(SIZE_SKIN.transposed().width()
            - fit::fx(44), fit::fx(27), fit::fx(20), fit::fx(20));
    }
}

void ExecutiveWidget::setWindow(QQuickWindow* window)
{
    window->hide();
    _contentItem = window->contentItem();
    _contentItem->setParentItem(_window.contentItem());
    _layout.update();

    /* Property sync between window and _window */
    auto mO = _window.metaObject();
    while (mO) {
        for (int i = mO->propertyOffset(); i < mO->propertyCount(); i++) {
            if (mO->property(i).isWritable() && mO->property(i).isReadable()) {
                _window.setProperty(mO->property(i).name(), mO->property(i).read(window));
            }
        }
        mO = mO->superClass();
    }
}

void ExecutiveWidget::stop()
{
    hide();
    if (_contentItem)
        _contentItem->setParentItem(nullptr);
    emit done();
}

void ExecutiveWidget::mousePressEvent(QMouseEvent *event)
{
    if (!_exitButton.geometry().contains(event->pos())) {
        _x = event->x(), _y = event->y();
        _pressed = true;
    }
    QWidget::mousePressEvent(event);
}

void ExecutiveWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (_pressed)
        move(event->globalX() - _x , event->globalY() - _y);
    QWidget::mouseMoveEvent(event);
}

void ExecutiveWidget::mouseReleaseEvent(QMouseEvent* event)
{
    _pressed = false;
    QWidget::mouseReleaseEvent(event);
}

void ExecutiveWidget::closeEvent(QCloseEvent* event)
{
    QWidget::closeEvent(event);
    stop();
}

void ExecutiveWidget::paintEvent(QPaintEvent* event)
{
    QRectF skinRect;
    QRectF innerRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    switch (_skin) {
        case Skin::PhonePortrait: {
            skinRect = QRectF({0, 0}, SIZE_SKIN);
            skinRect.moveCenter(innerRect.center());
            QSvgRenderer svg(QString(":/resources/images/phnv.svg"));
            svg.render(&painter, skinRect);
            break;
        }

        case Skin::PhoneLandscape: {
            skinRect = QRectF({0, 0}, SIZE_SKIN.transposed());
            skinRect.moveCenter(innerRect.center());
            QSvgRenderer svg(QString(":/resources/images/phnh.svg"));
            svg.render(&painter, skinRect);
            break;
        }

        default:
            break;
    }
    QWidget::paintEvent(event);
}
