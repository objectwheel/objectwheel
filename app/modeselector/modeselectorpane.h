#ifndef MODESELECTORPANE_H
#define MODESELECTORPANE_H

#include <QToolBar>

class QToolButton;
class ModeSelectorPane final : public QToolBar
{
    Q_OBJECT
    Q_DISABLE_COPY(ModeSelectorPane)

public:
    explicit ModeSelectorPane(QWidget* parent = nullptr);

    QToolButton* designerButton() const;
    QToolButton* editorButton() const;
    QToolButton* splitButton() const;
    QToolButton* optionsButton() const;
    QToolButton* buildsButton() const;
    QToolButton* documentsButton() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateIcons();
    void paintEvent(QPaintEvent* event) override;

private:
    QToolButton* m_designerButton;
    QToolButton* m_editorButton;
    QToolButton* m_splitButton;
    QToolButton* m_optionsButton;
    QToolButton* m_buildsButton;
    QToolButton* m_documentsButton;
};

#endif // MODESELECTORPANE_H