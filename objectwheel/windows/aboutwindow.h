#ifndef ABOUTWINDOW_H
#define ABOUTWINDOW_H

#include <QWidget>

class AboutWindow final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(AboutWindow)

public:
    explicit AboutWindow(QWidget* parent = nullptr);

public:
    QSize sizeHint() const override;

signals:
    void done();
};

#endif //  ABOUTWINDOW_H
