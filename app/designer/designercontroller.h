#ifndef DESIGNERCONTROLLER_H
#define DESIGNERCONTROLLER_H

#include <QObject>

class DesignerController : public QObject
{
    Q_OBJECT
public:
    explicit DesignerController(QObject *parent = nullptr);

signals:

public slots:
};

#endif // DESIGNERCONTROLLER_H
