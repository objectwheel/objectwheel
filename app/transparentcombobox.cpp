#include <transparentcombobox.h>

#include <utils/utilsicons.h>
#include <utils/theme/theme.h>

#include <QPainter>

using namespace Utils;
#include <transparentstyle.h>
#include <QLineEdit>
#include <QtWidgets>
TransparentComboBox::TransparentComboBox(QWidget* parent) : QComboBox(parent)
{
    setAttribute(Qt::WA_Hover);
    setStyle(new TransparentStyle(this));

    for (QObject* o : findChildren<QObject*>("")) {
        if (qobject_cast<QWidget*>(o)) {
            qobject_cast<QWidget*>(o)->setStyle(style());
//            qDebug() << o;
        }
    }
//    qDebug() << "----";
}

//void TransparentComboBox::paintEvent(QPaintEvent*)
//{
//    QPainter painter(this);
//    painter.setRenderHint(QPainter::Antialiasing);
//qDebug() << underMouse();
//    if (underMouse())
//        painter.fillRect(rect(), "#15000000");
//    else
//        painter.fillRect(rect(), Qt::transparent);

//    painter.setPen(isEnabled() ? Qt::black : Qt::gray);
//    painter.drawText(rect().adjusted(4, 0, -12, 0),
//                     fontMetrics().elidedText(currentText(), Qt::ElideRight, width() - 16),
//                     QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

//    QRectF arrowRect;
//    arrowRect.setSize({10.0, 10.0});
//    arrowRect.moveRight(width() - 2);

//    static const QPixmap up = Icon({{":/utils/images/arrowup.png",
//                                     Theme::ComboBoxArrowColor}}).pixmap();
//    static const QPixmap down = Icon({{":/utils/images/arrowdown.png",
//                                       Theme::ComboBoxArrowColor}}).pixmap();
//    static const QPixmap upDisabled = Icon({{":/utils/images/arrowup.png",
//                                             Theme::ComboBoxArrowColorDisabled}}).pixmap();
//    static const QPixmap downDisabled = Icon({{":/utils/images/arrowdown.png",
//                                               Theme::ComboBoxArrowColorDisabled}}).pixmap();

//    arrowRect.moveBottom(height() / 2.0);
//    painter.drawPixmap(arrowRect, isEnabled() ? up : upDisabled, up.rect());

//    arrowRect.moveTop(height() / 2.0);
//    painter.drawPixmap(arrowRect, isEnabled() ? down : downDisabled, down.rect());
//}