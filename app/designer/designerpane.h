#ifndef DESIGNERPANE_H
#define DESIGNERPANE_H

#include <QWidget>

class QMenu;
class QToolBar;
class DesignerView;
class SignalEditor;
class AnchorEditor;

class DesignerPane final : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(DesignerPane)

public:
    explicit DesignerPane(QWidget* parent = nullptr);

    QMenu* menu() const;
    QToolBar* toolBar() const;
    DesignerView* designerView() const;
    SignalEditor* signalEditor() const;
    AnchorEditor* anchorEditor() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void updateIcons();
    void changeEvent(QEvent* event) override;

private:
    QMenu* m_menu;
    QToolBar* m_toolBar;
    DesignerView* m_designerView;
    SignalEditor* m_signalEditor;
    AnchorEditor* m_anchorEditor;
};

#endif // DESIGNERPANE_H
