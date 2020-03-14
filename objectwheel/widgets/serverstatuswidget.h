#ifndef SERVERSTATUSWIDGET_H
#define SERVERSTATUSWIDGET_H

#include <QWidget>

class ServerStatusWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ServerStatusWidget)

public:
    explicit ServerStatusWidget(QWidget* parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void updateToolTip();

private:
    void paintEvent(QPaintEvent* event) override;
};

#endif // SERVERSTATUSWIDGET_H
