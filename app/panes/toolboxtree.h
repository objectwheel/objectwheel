#ifndef TOOLBOXTREE_H
#define TOOLBOXTREE_H

#include <QTreeWidget>
#include <QUrl>

class ToolboxTree : public QTreeWidget
{
    Q_OBJECT

public:
    explicit ToolboxTree(QWidget *parent = nullptr);

    void addUrls(QTreeWidgetItem* item, const QList<QUrl>& urls);
    void removeUrls(QTreeWidgetItem* item);
    const QMap<QTreeWidgetItem*, QList<QUrl>>& allUrls() const;
    QList<QUrl> urls(QTreeWidgetItem* item) const;

    bool contains(const QString& itemName);
    bool categoryContains(const QString& categoryName);
    QTreeWidgetItem* categoryItem(const QString& categoryName);

public slots:
    void clearUrls();

protected:
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    QPoint _pressPoint;
    QMap<QTreeWidgetItem*, QList<QUrl>> _urls;
};

#endif // TOOLBOXTREE_H
