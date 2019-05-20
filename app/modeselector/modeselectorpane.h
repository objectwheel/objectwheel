#ifndef MODESELECTORPANE_H
#define MODESELECTORPANE_H

#include <QToolBar>

class ModeSelectorPane final : public QToolBar
{
    Q_OBJECT
    Q_DISABLE_COPY(ModeSelectorPane)

public:
    explicit ModeSelectorPane(QWidget* parent = nullptr);

    QAction* designerAction() const;
    QAction* editorAction() const;
    QAction* splitAction() const;
    QAction* optionsAction() const;
    QAction* buildsAction() const;
    QAction* helpAction() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateIcons();
    bool event(QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent*) override;

private:
    QAction* m_designerAction;
    QAction* m_editorAction;
    QAction* m_splitAction;
    QAction* m_optionsAction;
    QAction* m_buildsAction;
    QAction* m_documentsAction;
};

#endif // MODESELECTORPANE_H