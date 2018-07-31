#include <applicationstyle.h>

#include <QStyleFactory>
#include <QApplication>

ApplicationStyle::ApplicationStyle() : QProxyStyle("fusion")
{
    Q_ASSERT(QStyleFactory::keys().contains("fusion", Qt::CaseInsensitive));
}
