#ifndef BUILDDETAILSDIALOG_H
#define BUILDDETAILSDIALOG_H

#include <QDialog>
#include <QModelIndex>

class QPlainTextEdit;
class QAbstractItemView;
class DownloadDetailsWidget;

class BuildDetailsDialog final : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(BuildDetailsDialog)

public:
    explicit BuildDetailsDialog(const QAbstractItemView* view, QWidget* parent = nullptr);
    ~BuildDetailsDialog() override;

    const QModelIndex& index() const;
    void setIndex(const QModelIndex& index);

private slots:
    void onModelReset();
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    void highlight(int begin);

private:
    const QAbstractItemView* m_view;
    DownloadDetailsWidget* m_downloadDetailsWidget;
    QPlainTextEdit* m_detailsTextEdit;
    QModelIndex m_index;
};

#endif // BUILDDETAILSDIALOG_H
