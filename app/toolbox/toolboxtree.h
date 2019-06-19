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

    void addTool(const QString& name, const QString& category,
                 const QString& dir, const QIcon& icon);

private:
    ToolboxItem* categoryItem(const QString& category);

private slots:
    void onMousePress(QTreeWidgetItem* item, int column);

signals:
    void pressed(ToolboxItem* item);
};

#endif // TOOLBOXTREE_H
