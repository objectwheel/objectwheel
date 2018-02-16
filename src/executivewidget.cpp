#include <executivewidget.h>
#include <fit.h>
#include <QPainter>
#include <QSvgRenderer>
#include <QQuickItem>
#include <QMouseEvent>
#include <QStyle>
#include <QApplication>
#include <QScreen>

ExecutiveWidget::ExecutiveWidget(QWidget* parent) : QWidget(parent)
  , _layout(this)
  , _x(0), _y(0)
  , _pressed(false)
{
    #if !defined(Q_OS_WIN)
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    #endif
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint );
    setAttribute(Qt::WA_QuitOnClose, false);

    _layout.setSpacing(0);
    _layout.setContentsMargins(0, 0, 0, 0);

    _containerWidget = createWindowContainer(&_window);
    _layout.addWidget(_containerWidget);
    _layout.setAlignment(_containerWidget, Qt::AlignCenter);
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
    } else {
        setFixedSize(SIZE_SKIN.transposed());
        _containerWidget->setFixedSize(SIZE_FORM.transposed());
    }
}

void ExecutiveWidget::setWindow(QQuickWindow* window)
{
    window->close();
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

    setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            size(),
            qApp->primaryScreen()->availableGeometry()
        )
    );
}

void ExecutiveWidget::stop()
{
    hide();
    if (_contentItem)
        _contentItem->setParentItem(nullptr);
    emit done();
}

void ExecutiveWidget::mousePressEvent(QMouseEvent* event)
{
    _x = event->x(), _y = event->y();
    _pressed = true;
    QWidget::mousePressEvent(event);
}

void ExecutiveWidget::mouseMoveEvent(QMouseEvent* event)
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
