#ifndef TOOLBOXPANE_H
#define TOOLBOXPANE_H

#include <QWidget>

class QVBoxLayout;
class ToolboxTree;
class LineEdit;
class QTreeWidgetItem;
class MainWindow;

class ToolboxPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxPane)

public:
    explicit ToolboxPane(QWidget* parent = nullptr);
    ToolboxTree* toolboxTree();

public slots:
    void discharge();

protected:
    QSize sizeHint() const override;

private slots:
    void fillPane();
    void filterList(const QString& filter);

signals:
    void filled();
    void itemDoubleClicked(const QString& url);

private:
    QVBoxLayout* _layout;
    ToolboxTree* _toolboxTree;
    LineEdit* _searchEdit;
};

#endif // TOOLBOXPANE_H
