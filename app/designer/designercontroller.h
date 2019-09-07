#ifndef DESIGNERCONTROLLER_H
#define DESIGNERCONTROLLER_H

#include <QObject>

class DesignerPane;
class DesignerController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerController)

public:
    explicit DesignerController(DesignerPane* designerPane, QObject* parent = nullptr);

public slots:
    void discharge();

private slots:

private:
    DesignerPane* m_designerPane;
};

#endif // DESIGNERCONTROLLER_H
