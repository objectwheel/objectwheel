#ifndef DESIGNERWIDGET_H
#define DESIGNERWIDGET_H

#include <QWidget>

class QmlCodeEditorWidget;
class DesignerScene;
class DesignerView;
class QVBoxLayout;
class ToolBar;
class ToolButton;
class QComboBox;
class Control;

class DesignerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DesignerWidget(QmlCodeEditorWidget* qmlCodeEditorWidget, QWidget* parent = nullptr);
    DesignerScene* designerScene() const;

public slots:
    void reset();
    void refresh() { onRefreshButtonClick(); }

protected:
    QSize sizeHint() const override;

private slots:
    void onFitButtonClick();
    void onUndoButtonClick();
    void onRedoButtonClick();
    void onClearButtonClick();
    void onRefreshButtonClick();
    void onOutlineButtonClick(bool value);
    void onSnappingButtonClick(bool value);
    void onZoomLevelChange(const QString& text);

    void onControlClick(Control*);
    void onControlDoubleClick(Control*);
    void onControlDrop(Control*, const QPointF&, const QString&);

private:
    void scaleScene(qreal ratio);

private:
    qreal m_lastScale;
    QmlCodeEditorWidget* m_qmlCodeEditorWidget;
    QVBoxLayout* m_layout;
    DesignerScene* m_designerScene;
    DesignerView* m_designerView;
    ToolBar* m_toolbar;
    ToolButton* m_undoButton;
    ToolButton* m_redoButton;
    ToolButton* m_clearButton;
    ToolButton* m_refreshButton;
    ToolButton* m_snappingButton;
    ToolButton* m_fitButton;
    ToolButton* m_outlineButton;
    QComboBox* m_zoomlLevelCombobox;
};

#endif // DESIGNERWIDGETm_H
