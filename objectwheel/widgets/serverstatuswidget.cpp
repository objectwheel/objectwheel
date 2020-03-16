#include <serverstatuswidget.h>
#include <servermanager.h>
#include <utilityfunctions.h>
#include <QPainter>

ServerStatusWidget::ServerStatusWidget(QWidget* parent) : QWidget(parent)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    connect(ServerManager::instance(), &ServerManager::stateChanged,
            this, &ServerStatusWidget::updateToolTip);
    connect(ServerManager::instance(), &ServerManager::stateChanged,
            this, qOverload<>(&ServerStatusWidget::update));
}

QSize ServerStatusWidget::sizeHint() const
{
    return minimumSizeHint();
}

QSize ServerStatusWidget::minimumSizeHint() const
{
    return QSize(12, 12);
}

void ServerStatusWidget::updateToolTip()
{
    static const QString toolTipTemplate(tr("<p style='white-space:nowrap'>Server connection status: <b>%1</b></p>"));
    const QAbstractSocket::SocketState state = ServerManager::instance()->state();
    if (state == QAbstractSocket::ConnectedState)
        UtilityFunctions::updateToolTip(this, toolTipTemplate.arg(tr("Connected")));
    else if (state == QAbstractSocket::UnconnectedState)
        UtilityFunctions::updateToolTip(this, toolTipTemplate.arg(tr("Disconnected")));
    else
        UtilityFunctions::updateToolTip(this, toolTipTemplate.arg(tr("Connecting...")));
}

void ServerStatusWidget::paintEvent(QPaintEvent*)
{
    const QAbstractSocket::SocketState state = ServerManager::instance()->state();
    const QColor color(state != QAbstractSocket::ConnectedState
            ? state == QAbstractSocket::UnconnectedState ? "#f78f8f" : "#f6e69d" : "#b9debc");
    const QColor outline(state != QAbstractSocket::ConnectedState
            ? state == QAbstractSocket::UnconnectedState ? "#c74343" : "#b59746" : "#5d9b75");
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(outline);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), width() / 2.0, height() / 2.0);
}
