#ifndef DOCKBAR_H
#define DOCKBAR_H

#include <QToolBar>

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
    void removeDockWidget(QDockWidget* dockWidget);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void paintEvent(QPaintEvent* event) override;

signals:
    void dockWidgetShown(QDockWidget* dockWidget);

private:
    QList<DockData> m_dockDataList;
};

#endif // DOCKBAR_H
