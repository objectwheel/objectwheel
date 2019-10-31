#ifndef FORMSPANE_H
#define FORMSPANE_H

#include <QWidget>

class FormsTree;
class LineEdit;
class QPushButton;

class FormsPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(FormsPane)

public:
    explicit FormsPane(QWidget* parent = nullptr);

    FormsTree* formsTree() const;
    LineEdit* searchEdit() const;
    QPushButton* addButton() const;
    QPushButton* removeButton() const;
    QPushButton* renameButton() const;
    QSize sizeHint() const override;

private:
    FormsTree* m_formsTree;
    LineEdit* m_searchEdit;
    QPushButton* m_addButton;
    QPushButton* m_removeButton;
    QPushButton* m_renameButton;
};

#endif // FORMSPANE_H
