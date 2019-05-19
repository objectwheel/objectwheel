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
    QToolButton* splitViewButton() const;
    QToolButton* optionsButton() const;
    QToolButton* buildsButton() const;
    QToolButton* helpButton() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateIcons();
    bool event(QEvent* event) override;
    void changeEvent(QEvent* event) override;
    void paintEvent(QPaintEvent*) override;

private:
    QToolButton* m_designerButton;
    QToolButton* m_editorButton;
    QToolButton* m_splitViewButton;
    QToolButton* m_optionsButton;
    QToolButton* m_buildsButton;
    QToolButton* m_helpButton;
};

#endif // MODESELECTORPANE_H