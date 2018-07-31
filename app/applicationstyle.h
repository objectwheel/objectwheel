#ifndef APPLICATIONSTYLE_H
#define APPLICATIONSTYLE_H

#include <QProxyStyle>

class ApplicationStyle : public QProxyStyle
{
    Q_OBJECT
    Q_DISABLE_COPY(ApplicationStyle)

public:
    ApplicationStyle();
};

#endif // APPLICATIONSTYLE_H