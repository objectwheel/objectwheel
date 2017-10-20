#include <executivewidget.h>
#include <fit.h>
#include <QPainter>
#include <QSvgRenderer>
#include <QQmlEngine>
#include <QMouseEvent>

using namespace Fit;

ExecutiveWidget::ExecutiveWidget(QWidget *parent)
    : QWidget(parent)
    , _layout(this)
    , _exitButton(this)
    , _x(0), _y(0)
    , _pressed(false)
{
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    _exitButton.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    _exitButton.setColor("#D11919");
    _exitButton.setFixedSize(fit(20),fit(20));
    _exitButton.setRadius(fit(9));
    _exitButton.setIconSize(QSize(fit(17),fit(17)));
    _exitButton.setIcon(QIcon(":/resources/images/down-arrow.png"));
    connect(&_exitButton, SIGNAL(clicked(bool)), SLOT(handleExitButtonClick()));

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
        _exitButton.setGeometry(SIZE_SKIN.width() - fit(55),
            fit(26), fit(20), fit(20));
    } else {
        setFixedSize(SIZE_SKIN.transposed());
        _exitButton.setGeometry(SIZE_SKIN.transposed().width()
            - fit(44), fit(27), fit(20), fit(20));
    }
}
#include <QQuickWindow>
#include <QQuickItem>
#include <QTimer>
#include <QQuickView>
void ExecutiveWidget::setData(QQmlEngine* engine, QQuickWindow* window)
{
    QQuickView* view = new QQuickView;
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    _engine = engine;
    _containerWidget = createWindowContainer(view);
    _containerWidget->setFixedSize(SIZE_FORM);
    _layout.addWidget(_containerWidget);
    window->contentItem()->setParentItem(view->contentItem());
    window->contentItem()->setPosition({0, 0});
    //TODO
}

void ExecutiveWidget::handleExitButtonClick()
{
    hide();
    _engine->deleteLater();
    _containerWidget->deleteLater();
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

void ExecutiveWidget::paintEvent(QPaintEvent* event)
{
    QRectF skinRect;
    QRectF innerRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainter painter(this);
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
