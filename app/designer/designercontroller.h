#ifndef DESIGNERCONTROLLER_H
#define DESIGNERCONTROLLER_H

#include <QObject>

class DesignerPane;
class Control;

class DesignerController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerController)

public:
    explicit DesignerController(DesignerPane* designerPane, QObject* parent = nullptr);

public slots:
    void charge();
    void discharge();

private slots:
    void onCustomContextMenuRequest(const QPoint& pos);

private:
    DesignerPane* m_designerPane;
    Control* m_menuTargetControl;
};

#endif // DESIGNERCONTROLLER_H
