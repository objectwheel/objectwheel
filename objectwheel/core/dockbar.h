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

    bool buttonExists(QDockWidget* dockWidget) const;
    void addDockWidget(QDockWidget* dockWidget);
    void removeDockWidget(QDockWidget* dockWidget);
    void setDockWidgetButtonChecked(QDockWidget* dockWidget, bool checked) const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void paintEvent(QPaintEvent* event) override;

signals:
    void dockWidgetButtonClicked(QDockWidget* dockWidget, bool checked);

private:
    QList<DockData> m_dockDataList;
};

#endif // DOCKBAR_H