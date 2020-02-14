#ifndef DOWNLOADCONTROLLER_H
#define DOWNLOADCONTROLLER_H

#include <QObject>

class DownloadWidget;
class DownloadController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DownloadController)

public:
    explicit DownloadController(DownloadWidget* downloadWidget, QObject* parent = nullptr);

private:
    DownloadWidget* m_downloadWidget;
};

#endif // DOWNLOADCONTROLLER_H
