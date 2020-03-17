#ifndef DOWNLOADCONTROLLER_H
#define DOWNLOADCONTROLLER_H

#include <QObject>

class DownloadWidget;
class BuildDetailsDialog;

class DownloadController final : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DownloadController)

public:
    explicit DownloadController(DownloadWidget* downloadWidget, QObject* parent = nullptr);

private slots:
    void onInfoButtonClick(const QModelIndex& index) const;
    void onDeleteButtonClick(const QModelIndex& index) const;
    void onOpenFolderButtonClick(const QModelIndex& index) const;

private:
    DownloadWidget* m_downloadWidget;
    BuildDetailsDialog* m_buildDetailsDialog;
};

#endif // DOWNLOADCONTROLLER_H
