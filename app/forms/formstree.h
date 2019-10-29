#ifndef FORMSTREE_H
#define FORMSTREE_H

#include <QTreeWidget>

class FormsDelegate;
class FormsTree final : public QTreeWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FormsTree)
    Q_DECLARE_PRIVATE(QTreeWidget)

public:
    explicit FormsTree(QWidget* parent = nullptr);

    FormsDelegate* delegate() const;

private:
    void paintEvent(QPaintEvent* event) override;

private:
    FormsDelegate* m_delegate;
};

#endif // FORMSTREE_H
