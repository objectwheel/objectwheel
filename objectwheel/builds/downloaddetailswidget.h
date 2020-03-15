#ifndef DOWNLOADDETAILSWIDGET_H
#define DOWNLOADDETAILSWIDGET_H

#include <QAbstractItemView>

class DownloadDetailsWidget final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DownloadDetailsWidget)

public:
    explicit DownloadDetailsWidget(const QAbstractItemView* view, QWidget* parent = nullptr);
    ~DownloadDetailsWidget() override;

    const QModelIndex& index() const;
    void setIndex(const QModelIndex& index);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QStyleOptionViewItem viewOptions() const;

private:
    const QAbstractItemView* m_view;
    QModelIndex m_index;
    QWidget* m_editor;
};

#endif // DOWNLOADDETAILSWIDGET_H
