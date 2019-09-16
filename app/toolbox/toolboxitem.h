#ifndef TOOLBOXITEM_H
#define TOOLBOXITEM_H

#include <QTreeWidgetItem>

class ToolboxItem final : public QTreeWidgetItem
{
public:
    enum ItemType { Type = UserType + 1 };

    ToolboxItem(int type = Type);

    bool isCategory() const;

    QIcon icon() const;

    QString dir() const;
    void setDir(const QString& dir);

    bool operator<(const QTreeWidgetItem& other) const override;

private:
    QString m_dir;
};

#endif // TOOLBOXITEM_H
