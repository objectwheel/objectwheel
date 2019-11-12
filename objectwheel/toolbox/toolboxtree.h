#ifndef TOOLBOXTREE_H
#define TOOLBOXTREE_H

#include <QTreeWidget>

class ToolboxItem;
class ToolboxTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxTree)

public:
    explicit ToolboxTree(QWidget* parent = nullptr);

    ToolboxItem* addTool(const QString& dir);

private:
    ToolboxItem* categoryItem(const QString& category);

private slots:
    void onItemPress(QTreeWidgetItem* item, int column);

signals:
    void itemPressed(ToolboxItem* item);
};

#endif // TOOLBOXTREE_H
