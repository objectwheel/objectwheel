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

    QByteArray identifier() const;
    void setIdentifier(const QByteArray& identifier);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private slots:
    void onModelReset();
    void onRowsAboutToBeRemoved(const QModelIndex& parent, int first, int last);
    void onDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    QModelIndex index() const;

private:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QStyleOptionViewItem viewOptions() const;

private:
    const QAbstractItemView* m_view;
    QByteArray m_identifier;
    QWidget* m_editor;
};

#endif // DOWNLOADDETAILSWIDGET_H
