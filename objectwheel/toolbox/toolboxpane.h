#ifndef TOOLBOXPANE_H
#define TOOLBOXPANE_H

#include <QWidget>

class LineEdit;
class ToolboxTree;

class ToolboxPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ToolboxPane)

public:
    explicit ToolboxPane(QWidget* parent = nullptr);

    LineEdit* searchEdit() const;
    ToolboxTree* toolboxTree() const;

    QSize sizeHint() const override;

private:
    void dragEnterEvent(QDragEnterEvent* event) override;

private:
    LineEdit* m_searchEdit;
    ToolboxTree* m_toolboxTree;
};

#endif // TOOLBOXPANE_H
