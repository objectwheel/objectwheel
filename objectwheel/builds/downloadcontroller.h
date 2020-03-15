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

private slots:
    void onDeleteButtonClick(const QModelIndex& index) const;
    void onOpenFolderButtonClick(const QModelIndex& index) const;
    void onModelDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight,
                           const QVector<int>& roles) const;

private:
    DownloadWidget* m_downloadWidget;
};

#endif // DOWNLOADCONTROLLER_H
