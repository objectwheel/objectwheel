#ifndef NAVIGATORPANE_H
#define NAVIGATORPANE_H

#include <QWidget>

class NavigatorTree;
class LineEdit;

class NavigatorPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(NavigatorPane)

public:
    explicit NavigatorPane(QWidget* parent = nullptr);

    NavigatorTree* navigatorTree() const;
    LineEdit* searchEdit() const;
    QSize sizeHint() const override;

private:
    NavigatorTree* m_navigatorTree;
    LineEdit* m_searchEdit;
};

#endif // NAVIGATORPANE_H
