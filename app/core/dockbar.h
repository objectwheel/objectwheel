#ifndef DOCKBAR_H
#define DOCKBAR_H

#include <QToolBar>

class QAction;
class QDockWidget;

class DockBar final : public QToolBar
{
    Q_OBJECT
    Q_DISABLE_COPY(DockBar)

    struct DockData {
        QAction* action;
        QDockWidget* dockWidget;
    };

public:
    explicit DockBar(QWidget* parent = nullptr);

    void addDockWidget(QDockWidget* dockWidget);

//    QSize sizeHint() const override;
//    QSize minimumSizeHint() const override;

private:
    QList<DockData> m_dockWidgets;
};

#endif // DOCKBAR_H
