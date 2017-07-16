#include <bubblehead.h>
#include <fit.h>
#include <QPainter>
#include <QMouseEvent>
#include <QIcon>
#include <QGraphicsDropShadowEffect>
#include <mainwindow.h>
#include <QVariantAnimation>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QTimer>
#include <QDebug>
#include <QtMath>

#if defined(Q_OS_IOS) || defined(Q_OS_ANDROID) || defined(Q_OS_WINPHONE)
#define MOBILE_FACTOR 1.3
#else
#define MOBILE_FACTOR 1.0
#endif
#define SMALL_SIZE (48.0 * (MOBILE_FACTOR))
#define BIG_SIZE (96.0 * (MOBILE_FACTOR))
#define DURATION 300

using namespace Fit;

class BubbleHeadPrivate
{
	public:
        BubbleHeadPrivate(BubbleHead* p);
        void fixCoord();
        void handleClick();
        void spin(bool up);

        BubbleHead* parent;
        QPoint hotPoint;
        QPixmap icon;
        QGraphicsDropShadowEffect shadowEffect;
        bool moved;

        QPainterPath centerPath;
        QParallelAnimationGroup animDriver;
        QPropertyAnimation moveAnim;
        QVariantAnimation rotateAnim;
        QVariantAnimation rotateAnim2;
        QVariantAnimation centerAnim;
};

BubbleHeadPrivate::BubbleHeadPrivate(BubbleHead* p)
	: parent(p)
{
	moved = false;
    shadowEffect.setBlurRadius(6);
    shadowEffect.setOffset(0, fit(1));
    shadowEffect.setColor("#65000000");
	parent->setGraphicsEffect(&shadowEffect);
	parent->setCursor(Qt::PointingHandCursor);
    parent->setCheckable(true);
    parent->resize(fit(SMALL_SIZE), fit(SMALL_SIZE));
    parent->updatePathes();
    moveAnim.setTargetObject(parent);
    moveAnim.setPropertyName("geometry");
    animDriver.addAnimation(&rotateAnim);
    animDriver.addAnimation(&rotateAnim2);
    animDriver.addAnimation(&moveAnim);
    animDriver.addAnimation(&centerAnim);
    moveAnim.setEasingCurve(QEasingCurve::OutQuart);
    rotateAnim.setEasingCurve(QEasingCurve::OutQuart);
    rotateAnim2.setEasingCurve(QEasingCurve::OutQuart);
    centerAnim.setEasingCurve(QEasingCurve::OutQuart);
    centerAnim.setKeyValueAt(0, fit(SMALL_SIZE / 2.0));
    QObject::connect(&centerAnim, &QVariantAnimation::valueChanged, [this]{parent->updatePathes();});
    QObject::connect(&rotateAnim, &QVariantAnimation::valueChanged, [this]{parent->updatePathes();});
    QObject::connect(&rotateAnim2, &QVariantAnimation::valueChanged, [this]{parent->updatePathes();});
    QObject::connect(&moveAnim, &QPropertyAnimation::valueChanged, [this]{parent->updatePathes();});
    QObject::connect(&moveAnim, &QPropertyAnimation::finished, [this]{fixCoord();});
	QObject::connect((MainWindow*)parent->parent(), &MainWindow::resized, [this]{fixCoord();});
    QObject::connect(parent, &BubbleHead::clicked, [this]{handleClick();});
    parent->updatePathes();
}

void BubbleHeadPrivate::fixCoord()
{
	QWidget* grandParent = (QWidget*)(parent->parent());
	QRect grandParentRect = grandParent->rect().adjusted(0, 0, -parent->width(), -parent->height());
	QPoint newPos = parent->pos();
	if (newPos.x() < 0) newPos.setX(0);
	if (newPos.y() < 0) newPos.setY(0);
	if (newPos.x() > grandParentRect.width()) newPos.setX(grandParentRect.width());
	if (newPos.y() > grandParentRect.height()) newPos.setY(grandParentRect.height());
	parent->move(newPos);
	emit parent->moved(newPos+QPoint(parent->width()/2.0, parent->height()/2.0));
}

BubbleHeadPrivate* BubbleHead::m_d = nullptr;

BubbleHead::BubbleHead(QWidget *parent)
	: QPushButton(parent)
{
    if (m_d) return;
    m_d = new BubbleHeadPrivate(this);
}

BubbleHead::~BubbleHead()
{
    delete m_d;
}

void BubbleHead::setIcon(const QPixmap& icon)
{
	m_d->icon = icon;
    if (icon.devicePixelRatio() != qApp->devicePixelRatio())
        m_d->icon.setDevicePixelRatio(qApp->devicePixelRatio());
}

const QPixmap& BubbleHead::icon()
{
	return m_d->icon;
}

void BubbleHead::mousePressEvent(QMouseEvent* event)
{
	m_d->moved = false;
	m_d->hotPoint = event->pos();
	QPushButton::mousePressEvent(event);
}

void BubbleHead::mouseMoveEvent(QMouseEvent* event)
{
	if ((m_d->hotPoint - event->pos()).manhattanLength() <
		QApplication::startDragDistance()) return;
	event->accept();
	m_d->moved = true;
	QWidget* parent = (QWidget*)(BubbleHead::parent());
	QRect parentRect = parent->rect().adjusted(0, 0, -width(), -height());
	QPoint newPos = event->windowPos().toPoint() - m_d->hotPoint;
	if (newPos.x() < 0) newPos.setX(0);
	if (newPos.y() < 0) newPos.setY(0);
	if (newPos.x() > parentRect.width()) newPos.setX(parentRect.width());
	if (newPos.y() > parentRect.height()) newPos.setY(parentRect.height());
	move(newPos);
	emit moved(newPos+QPoint(width()/2.0, height()/2.0));
}

void BubbleHead::mouseReleaseEvent(QMouseEvent* event)
{
	if (!m_d->moved) {
		QPushButton::mouseReleaseEvent(event);
	} else {
		event->accept();
		setDown(false);
    }
}

void BubbleHead::updatePathes()
{
    if (m_buttonList.size() < 1) return;
    const qreal step = 360.0 / m_buttonList.size();
    for (int  i = 0; i < m_buttonList.size(); i++) {
        QPainterPath path;
        path.moveTo(QRectF(rect()).center());
        path.arcTo(QRectF(rect()), m_d->rotateAnim.currentValue().toInt() + 90 + step * i, step);
        path.closeSubpath();
        m_buttonList[i]->path = path;
    }

    QPainterPath path;
    auto centerCal = m_d->centerAnim.currentValue().isValid() ? m_d->centerAnim.currentValue().toReal() : (fit(SMALL_SIZE) / 2.0 - fit(0.5));
    path.addEllipse(QRectF(rect()).center(), centerCal, centerCal);
    m_d->centerPath = path;

    update();
}

void BubbleHead::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    QPen pen;
    painter.setRenderHint(QPainter::Antialiasing);

    if (width() > fit(SMALL_SIZE + 2)) {
        pen.setWidthF(fit(1));
        pen.setColor(QColor("#15404447"));
        painter.setPen(pen);
        painter.setBrush(Qt::NoBrush);
        for (auto button: m_buttonList) {
            painter.setClipPath(button->path);
            QPixmap p(button->icon.scaled(button->path.boundingRect().toRect().size() * qApp->devicePixelRatio(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
            painter.drawPixmap(button->path.boundingRect().toRect(), p);
            painter.drawPath(button->path);
        }
     }

    if (m_d->rotateAnim2.state() == QVariantAnimation::Running) {
        QTransform tr;
        tr.translate(width() / 2.0, width() / 2.0);
        tr.rotate(m_d->rotateAnim2.currentValue().toInt());
        tr.translate(-width() / 2.0, -width() / 2.0);
        painter.setTransform(tr);
    }

    painter.setClipping(false);
    pen.setColor(QColor("#0D74C8").darker(109));
    painter.setPen(pen);
    QLinearGradient gradient(m_d->centerPath.boundingRect().toRect().center().x(), m_d->centerPath.boundingRect().toRect().topLeft().y(),
                             m_d->centerPath.boundingRect().toRect().center().x(), m_d->centerPath.boundingRect().toRect().bottomLeft().y());
    gradient.setColorAt(0, QColor("#0D74C8").lighter(106));
    gradient.setColorAt(1, QColor("#0D74C8").darker(106));
    painter.setBrush(gradient);
    painter.drawPath(m_d->centerPath);

    auto rectArea = m_d->centerPath.boundingRect().toRect().adjusted(width() > fit(SMALL_SIZE + 2) ? 0 : 1,
                                                                     width() > fit(SMALL_SIZE + 2) ? 0 : 1,
                                                                     width() > fit(SMALL_SIZE + 2) ? -1 : 0,
                                                                     width() > fit(SMALL_SIZE + 2) ? -1 : 0);
    painter.drawPixmap(rectArea, m_d->icon.scaled(rectArea.size() * qApp->devicePixelRatio(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
}

void BubbleHead::moveEvent(QMoveEvent* event)
{
    QWidget::moveEvent(event);
    emit moved(event->pos()+QPoint(width()/2.0, height()/2.0));
}

void BubbleHeadPrivate::handleClick()
{
    if (parent->isChecked()) {
        spin(true);
    } else {
        spin(false);
        if (centerPath.contains(parent->mapFromGlobal(QCursor::pos()))) return;
        for (auto button : parent->m_buttonList) {
            if (button->path.contains(parent->mapFromGlobal(QCursor::pos()))) {
                button->call();
            }
        }
    }
}

void BubbleHeadPrivate::spin(bool up)
{
    animDriver.stop();
    if (up) {
        moveAnim.setDuration(DURATION);
        centerAnim.setDuration(DURATION);
        rotateAnim.setDuration(DURATION);
        rotateAnim2.setDuration(6 * DURATION);

        centerAnim.setStartValue(fit(SMALL_SIZE) / 2.0 - fit(0.5));
        centerAnim.setEndValue(fit(SMALL_SIZE) / 4.0 - fit(0.5));

        rotateAnim.setStartValue(360);
        rotateAnim.setEndValue(0);
        rotateAnim2.setStartValue(720);
        rotateAnim2.setEndValue(0);

        moveAnim.setStartValue(parent->geometry());
        moveAnim.setEndValue(QRect(parent->x() - fit(BIG_SIZE - SMALL_SIZE)/2.0,
                                   parent->y() - fit(BIG_SIZE - SMALL_SIZE)/2.0,
                                   fit(BIG_SIZE), fit(BIG_SIZE)));
        animDriver.start();
    } else {
        moveAnim.setDuration(DURATION);
        centerAnim.setDuration(DURATION);
        rotateAnim.setDuration(DURATION);
        rotateAnim2.setDuration(6 * DURATION);

        centerAnim.setStartValue(fit(SMALL_SIZE) / 4.0 - fit(0.5));
        centerAnim.setEndValue(fit(SMALL_SIZE) / 2.0 - fit(0.5));

        rotateAnim.setStartValue(0);
        rotateAnim.setEndValue(360);
        rotateAnim2.setStartValue(0);
        rotateAnim2.setEndValue(720);

        moveAnim.setStartValue(parent->geometry());
        moveAnim.setEndValue(QRect(parent->x() + fit(BIG_SIZE - SMALL_SIZE)/2.0,
                                   parent->y() + fit(BIG_SIZE - SMALL_SIZE)/2.0,
                                   fit(SMALL_SIZE), fit(SMALL_SIZE)));
        animDriver.start();
    }
}
