#ifndef DOWNLOADWIDGET_H
#define DOWNLOADWIDGET_H

#include <QWidget>

class QListWidget;
class DownloadWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DownloadWidget)

public:
    explicit DownloadWidget(QWidget* parent = nullptr);

    QListWidget* platformList() const;

private:
    QListWidget* m_platformList;
};

#endif // DOWNLOADWIDGET_H
