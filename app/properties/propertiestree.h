#ifndef PROPERTIESTREE_H
#define PROPERTIESTREE_H

#include <QTreeWidget>

class PropertiesTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(PropertiesTree)

    friend class PropertiesDelegate; // For itemFromIndex()

public:
    explicit PropertiesTree(QWidget* parent = nullptr);

};

#endif // PROPERTIESTREE_H
